#include <cmath>
#include <cstring>
#include <limits>

#include <gl/glew.h>

#include <VGUI_App.h>
#include <VGUI_Panel.h>
#include <VGUI_BitmapTGA.h>
#include <VGUI_ImagePanel.h>
#include <VGUI1/VGUI_RDBitmapTGA.h>
#include <VGUI_Label.h>

#include "Platform.h"

#include "CNetworkBuffer.h"
#include "Common.h"
#include "Engine.h"
#include "FilePaths.h"
#include "IMetaLoader.h"
#include "interface.h"
#include "Logging.h"
#include "steam/SteamWrapper.h"

#include "FileSystem2.h"
#include "CFileSystemWrapper.h"

#include "VGUI1/vgui_loadtga.h"

#include "font/FontRendering.h"

#include "ui/vgui1/vgui_SchemeManager.h"

#include "CEngine.h"

EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CEngine, IMetaTool, DEFAULT_IMETATOOL_NAME, g_Engine );

void CEngine::SetMyGameDir( const char* const pszGameDir )
{
	strncpy( m_szMyGameDir, pszGameDir, sizeof( m_szMyGameDir ) );
	m_szMyGameDir[ sizeof( m_szMyGameDir ) - 1 ] = '\0';
}

bool CEngine::Startup( IMetaLoader& loader, CreateInterfaceFn* pFactories, const size_t uiNumFactories )
{
	m_pLoader = &loader;

	if( !m_pLoader->GetGameDirectory( m_szMyGameDir, sizeof( m_szMyGameDir ) ) )
		return false;

	if( !( *m_szMyGameDir ) )
	{
		UTIL_ShowMessageBox( "No game directory set", "Error", LogType::ERROR );
		return false;
	}

	m_steam_api = Steam_LoadSteamAPI( filepaths::BIN_DIR );

	m_bSteamAPIInitialized = Steam_InitWrappers( m_steam_api, true );

	if( !m_bSteamAPIInitialized )
	{
		return false;
	}

	if( !g_SteamAPIContext.Init() )
	{
		UTIL_ShowMessageBox( "Failed to initialize Steam API Context. Exiting...\n", "Fatal Error", LogType::ERROR );
		return false;
	}

	for( size_t uiIndex = 0; uiIndex < uiNumFactories; ++uiIndex )
	{
		auto factory = pFactories[ uiIndex ];

		if( !g_pFileSystem )
		{
			g_pFileSystem = static_cast<IFileSystem2*>( factory( FILESYSTEM2_INTERFACE_VERSION, nullptr ) );
		}
	}

	if( !g_pFileSystem )
	{
		Msg( "Couldn't instantiate the filesystem\n" );
		return false;
	}

	if( !SetupFileSystem() )
	{
		Msg( "Failed to set up filesystem\n" );
		return false;
	}

	//Load the original filesystem and overwrite its filesystem's vtable with one that points to ours.
	//Note: if the original engine regains control, it might try to use preexisting handles. Don't let that happen. - Solokiller
	{
		CLibrary fileSystem;

		if( !fileSystem.Load( CLibArgs( "filesystem_stdio" ).DisablePrefixes( true ) ) )
		{
			Msg( "Couldn't load filesystem_stdio\n" );
			return false;
		}

		auto filesystemFactory = reinterpret_cast<CreateInterfaceFn>( fileSystem.GetFunctionAddress( CREATEINTERFACE_PROCNAME ) );

		if( !filesystemFactory )
		{
			Msg( "Couldn't find filesystem_stdio factory\n" );
			return false;
		}

		auto pFileSystem = static_cast<IFileSystem*>( filesystemFactory( FILESYSTEM_INTERFACE_VERSION, nullptr ) );

		if( !pFileSystem )
		{
			Msg( "Couldn't instantiate the filesystem from filesystem_stdio\n" );
			return false;
		}

		CFileSystemWrapper wrapper;

		//Don't try this at home.
		memcpy( pFileSystem, &wrapper, sizeof( IFileSystem ) );
	}

	if( !g_Video.Initialize() )
	{
		return false;
	}

	Msg( "HostInit\n" );

	if( !HostInit() )
	{
		UTIL_ShowMessageBox( "Error initializing host", "Fatal Error", LogType::ERROR );
		return false;
	}

	if( !g_FontManager.Initialize() )
	{
		return false;
	}

	return true;
}

bool CEngine::Run()
{
	return g_Video.Run( *this );
}

void CEngine::Shutdown()
{
	if( m_pSchemeManager )
	{
		delete m_pSchemeManager;
		m_pSchemeManager = nullptr;
	}

	g_FontManager.Shutdown();

	g_Video.Shutdown();

	if( m_steam_api.IsLoaded() )
	{
		if( m_bSteamAPIInitialized )
			SteamAPI_Shutdown();

		m_steam_api.Free();
	}
}

void CEngine::RunFrame()
{
	RenderFrame();
}

bool CEngine::SetupFileSystem()
{
	g_pFileSystem->AddSearchPath( ".", "ROOT" );

	//This will let us get files from the original game directory. - Solokiller
	g_pFileSystem->AddSearchPath( "../valve", "GAME" );

	//Not a typo, the current dir is added twice as both ROOT and BASE in this order. - Solokiller
	g_pFileSystem->AddSearchPath( ".", "BASE" );

	return true;
}

bool CEngine::HostInit()
{
	CNetworkBuffer::InitMasks();

	if( !g_CVar.Initialize() )
		return false;

	if( !g_CommandBuffer.Initialize( &g_CVar ) )
		return false;

	auto pApp = vgui::App::getInstance();

	pApp->reset();

	Scheme_Init();

	m_pSchemeManager = new CSchemeManager( g_Video.GetWidth(), g_Video.GetHeight() );

	m_pRootPanel = new vgui::Panel( 0, 0, g_Video.GetWidth(), g_Video.GetHeight() );

	m_pRootPanel->setPaintBorderEnabled( false );
	m_pRootPanel->setPaintBackgroundEnabled( false );
	m_pRootPanel->setPaintEnabled( false );

	vgui::Scheme* pScheme = pApp->getScheme();

	m_pRootPanel->setCursor( pScheme->getCursor( vgui::Scheme::scu_none ) );

	g_pVGUI1Surface = new CVGUI1Surface( m_pRootPanel );

	//Ripped from the SDK, this sets up reasonable defaults.
	int r, g, b, a;

	// primary text color
	// Get the colors
	//!! two different types of scheme here, need to integrate
	SchemeHandle_t hPrimaryScheme = m_pSchemeManager->getSchemeHandle( "Primary Button Text" );
	{
		// font
		pScheme->setFont( vgui::Scheme::sf_primary1, m_pSchemeManager->getFont( hPrimaryScheme ) );

		// text color
		m_pSchemeManager->getFgColor( hPrimaryScheme, r, g, b, a );
		pScheme->setColor( vgui::Scheme::sc_primary1, r, g, b, a );		// sc_primary1 is non-transparent orange

																	// background color (transparent black)
		m_pSchemeManager->getBgColor( hPrimaryScheme, r, g, b, a );
		pScheme->setColor( vgui::Scheme::sc_primary3, r, g, b, a );

		// armed foreground color
		m_pSchemeManager->getFgArmedColor( hPrimaryScheme, r, g, b, a );
		pScheme->setColor( vgui::Scheme::sc_secondary2, r, g, b, a );

		// armed background color
		m_pSchemeManager->getBgArmedColor( hPrimaryScheme, r, g, b, a );
		pScheme->setColor( vgui::Scheme::sc_primary2, r, g, b, a );

		//!! need to get this color from scheme file
		// used for orange borders around buttons
		m_pSchemeManager->getBorderColor( hPrimaryScheme, r, g, b, a );
		// pScheme->setColor(Scheme::sc_secondary1, r, g, b, a );
		pScheme->setColor( vgui::Scheme::sc_secondary1, static_cast<int>( 255 * 0.7 ), static_cast<int>( 170 * 0.7 ), 0, 0 );
	}

	// Change the second primary font (used in the scoreboard)
	SchemeHandle_t hScoreboardScheme = m_pSchemeManager->getSchemeHandle( "Scoreboard Text" );
	{
		pScheme->setFont( vgui::Scheme::sf_primary2, m_pSchemeManager->getFont( hScoreboardScheme ) );
	}

	// Change the third primary font (used in command menu)
	SchemeHandle_t hCommandMenuScheme = m_pSchemeManager->getSchemeHandle( "CommandMenu Text" );
	{
		pScheme->setFont( vgui::Scheme::sf_primary3, m_pSchemeManager->getFont( hCommandMenuScheme ) );
	}

	vgui::App::getInstance()->setScheme( pScheme );

	CreateMainMenu();

	return true;
}

#include <VGUI_Font.h>

void CEngine::CreateMainMenu()
{
	int wide, tall;

	m_pRootPanel->getSize( wide, tall );

	m_MainMenu = std::make_unique<CMainMenu>( m_pRootPanel, 0, 0, wide, tall );

	//TODO: test code, remove. - Solokiller
	std::unique_ptr<uint8_t[]> data;

	int size = 0;

	FileHandle_t file = g_pFileSystem->Open( "gfx/vgui/fonts/640_Scoreboard Title Text.tga", "rb" );

	if( file != FILESYSTEM_INVALID_HANDLE )
	{
		size = g_pFileSystem->Size( file );

		data = std::make_unique<uint8_t[]>( size );
		
		g_pFileSystem->Read( data.get(), size, file );

		g_pFileSystem->Close( file );
	}

	Msg( "Loaded bitmap font: %s\n", file != FILESYSTEM_INVALID_HANDLE ? "yes" : "no" );

	auto pFont = new vgui::Font( "Arial", data.get(), size, 16, 8, 0, 400, false, false, false, false );

	std::string szString = "Foobar\n";

	for( char ch = 'A'; ch <= 'Z'; ++ch )
		szString += ch;

	szString = "Here comes another essay length post!\nfont layout across multiple lines\n\t\tusing tabulation";

	auto pText = new vgui::Label( szString.c_str(), 0, 0 );

	pText->setParent( m_pRootPanel );

	pText->setBgColor( 0, 0, 0, 255 );
	pText->setFgColor( 255, 255, 255, 255 );

	auto pText2 = new vgui::Label( "Foobar", 0, 100 );

	pText2->setParent( m_pRootPanel );

	pText2->setBgColor( 0, 0, 0, 255 );
	pText2->setFgColor( 255, 255, 255, 255 );

	pText->setFont( pFont );
	pText2->setFont( pFont );

	pText->setText( szString.c_str() );
	pText2->setText( "Foobar" );
}

void CEngine::RenderFrame()
{
	RenderVGUI1();
}

void CEngine::RenderVGUI1()
{
	glClearColor( 0, 0, 0, 1 );

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glViewport( 0, 0, g_Video.GetWidth(), g_Video.GetHeight() );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	glOrtho( 0.0f, ( float ) g_Video.GetWidth(), ( float ) g_Video.GetHeight(), 0.0f, 1.0f, -1.0f );

	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();
	
	glDisable( GL_CULL_FACE );
	glDisable( GL_BLEND );
	glDisable( GL_DEPTH_TEST );
	glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	
	vgui::App::getInstance()->externalTick();
	m_pRootPanel->repaintAll();
	m_pRootPanel->paintTraverse();
	
	//g_pVGUI1Surface->swapBuffers();
	
	glPopMatrix();

	//glRectf( 0, 0, 100, 100 );
	
	//SDL_GL_SwapWindow( g_Video.GetWindow() );
}
