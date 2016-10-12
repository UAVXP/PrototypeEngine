#include <algorithm>

#include "steam/SteamWrapper.h"

#include "AddonInstaller.h"
#include "Common.h"
#include "FilePaths.h"
#include "FileSystem2.h"
#include "Logging.h"
#include "IMetaLoader.h"
#include "Helpers.h"
#include "CAddonDataLoader.h"

#include "CAddonInstaller.h"

EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CAddonInstaller, IMetaTool, DEFAULT_IMETATOOL_NAME, g_AddonInstaller );

//-----------------------------------------------------------------------------
// Purpose: callback hook for debug text emitted from the Steam API
//-----------------------------------------------------------------------------
extern "C" void __cdecl SteamAPIDebugTextHook( int nSeverity, const char *pchDebugText )
{
	// if you're running in the debugger, only warnings (nSeverity >= 1) will be sent
	// if you add -debug_steamapi to the command-line, a lot of extra informational messages will also be sent
#ifdef WIN32
	::OutputDebugString( pchDebugText );
#else
	printf( "%s", pchDebugText );
#endif

	if( nSeverity >= 1 )
	{
		// place to set a breakpoint for catching API errors
		int x = 3;
		x = x;
	}
}

bool CAddonInstaller::Startup( IMetaLoader& loader, CreateInterfaceFn* pFactories, const size_t uiNumFactories )
{
	m_pLoader = &loader;

	if( !m_pLoader->GetGameDirectory( m_szGameDir, sizeof( m_szGameDir ) ) )
		return false;

	if( !( *m_szGameDir ) )
	{
		UTIL_ShowMessageBox( "No game directory set", "Error", LogType::ERROR );
		return false;
	}

	if( !m_pLoader->GetToolDirectory( m_szWorkingDir, sizeof( m_szWorkingDir ) ) )
		return false;

	if( !( *m_szWorkingDir ) )
	{
		UTIL_ShowMessageBox( "No working directory set", "Error", LogType::ERROR );
		return false;
	}

	m_steam_api = Steam_LoadSteamAPI( filepaths::BIN_DIR );

	m_bSteamAPIInitialized = Steam_InitWrappers( m_steam_api, true );

	if( !m_bSteamAPIInitialized )
	{
		return false;
	}

	// set our debug handler
	SteamClient()->SetWarningMessageHook( &::SteamAPIDebugTextHook );

	if( !m_SteamAPIContext.Init() )
	{
		UTIL_ShowMessageBox( "Failed to initialize Steam API Context. Exiting...\n", "Fatal Error", LogType::ERROR );
		return false;
	}

	char szSCGameDir[ MAX_PATH ];

	if( m_SteamAPIContext.SteamApps()->GetAppInstallDir( SVENCOOP_APPID, szSCGameDir, sizeof( szSCGameDir ) ) )
	{
		m_SCGameDir = fs::path( szSCGameDir ) / "svencoop";
		m_SCGameDir.make_preferred();
	}
	else
	{
		Msg( "Couldn't get Sven Co-op install directory\n" );
	}

	for( size_t uiIndex = 0; uiIndex < uiNumFactories; ++uiIndex )
	{
		auto factory = pFactories[ uiIndex ];

		if( !m_pFileSystem )
		{
			m_pFileSystem = static_cast<IFileSystem2*>( factory( FILESYSTEM2_INTERFACE_VERSION, nullptr ) );
		}
	}

	if( !m_pFileSystem )
	{
		Msg( "Couldn't instantiate the filesystem\n" );
		return false;
	}

	return true;
}

bool CAddonInstaller::Run()
{
	const char* pszValue = nullptr;

	bool bSilent = false;

	if( auto pszValue = GetCommandLine()->GetValue( "-loglevel" ) )
	{
		int iLogLevel = atoi( pszValue );

		iLogLevel = std::min( static_cast<int>( LogLevel::LAST ), std::max( static_cast<int>( LogLevel::FIRST ), iLogLevel ) );

		g_LogLevel = static_cast<LogLevel>( iLogLevel );
	}

	/*
	Disabled until all code handles silent properly
	if( GetCommandLine()->HasArgument( "-silent" ) )
	{
		bSilent = true;
		g_LogLevel = LogLevel::SILENT;
	}
	*/

	Log( LogLevel::ALWAYS, "\n" );

	std::vector<CAppInfo> apps;

	{
		CAddonDataLoader loader;

		auto addonPath = fs::path( GetWorkingDirectory() ) / filepaths::RESOURCE_DIR / "addons.txt";

		if( loader.Load( addonPath.u8string().c_str() ) )
		{
			Log( LogLevel::NORMAL, "Loaded addon settings from file\n\n" );

			apps = std::move( loader.GetAppInfos() );
		}
	}

	if( !apps.empty() )
	{
		if( !GetDirectoriesFromSteam( apps ) )
			Log( LogLevel::ALWAYS, "Failed to get installation directories from Steam\n" );

		if( AskForDirectories( apps ) )
		{
			if( HasRequiredFiles() )
			{
				Log( LogLevel::NORMAL, "\nAll required installer files found\n\n" );

				for( const auto& appInfo : apps )
				{
					if( appInfo.szPath[ 0 ] )
					{
					
						if( !CopyGameFiles( appInfo ) )
							LogHelp( appInfo );
						
					}
				}

				Log( LogLevel::NORMAL, "\nDone!\n" );
			}
			else
			{
				Log( LogLevel::ALWAYS, "Unable to find some requires files\n" );
			}
		}
		else
			Log( LogLevel::ALWAYS, "Unable to find app install directories\n" );
	}
	else
		Log( LogLevel::ALWAYS, "No apps provided to copy\n" );

	if( !bSilent )
	{
		Log( LogLevel::ALWAYS, "Press Enter to continue..." );
		getchar();
	}

	// exit
	return true;
}

void CAddonInstaller::Shutdown()
{
	if( m_steam_api.IsLoaded() )
	{
		if( m_bSteamAPIInitialized )
			SteamAPI_Shutdown();

		m_steam_api.Free();
	}
}

void CAddonInstaller::LogHelp( const CAppInfo& appInfo )
{
	Log( LogLevel::ALWAYS, "Could not find game content for %s\n", appInfo.szName.c_str() );
	Log( LogLevel::ALWAYS, "Please verify that the game is installed, and that it has been run at least once\n" );
	Log( LogLevel::ALWAYS, "Verify that the path is correct\n" );
	Log( LogLevel::ALWAYS, "Verify the game's files through Steam\nRight-click the game->Properties->Local Files->Verify integrity of game cache\n" );
	Log( LogLevel::ALWAYS, "Verify that Sven Co-op's files are properly installed using the same method\n" );
	Log( LogLevel::ALWAYS, "Ensure that this program is being run from the Sven Co-op/svencoop/ directory\n" );
}
