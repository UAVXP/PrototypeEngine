#include <VGUI_ActionSignal.h>
#include <VGUI_Button.h>
#include <VGUI_ImagePanel.h>

#include "VGUI1/VGUI_RDBitmapTGA.h"
#include "VGUI1/vgui_loadtga.h"

#include "COptionsDialog.h"

#include "Engine.h"

#include "CMainMenu.h"

class COptionsActionSignal : public vgui::ActionSignal
{
public:
	COptionsActionSignal( CMainMenu* pMainMenu )
		: m_pMainMenu( pMainMenu )
	{
	}

	void actionPerformed( vgui::Panel* panel ) override
	{
		auto pDialog = m_pMainMenu->CreateOptionsDialog();
		pDialog->CenterOnParent();
	}

private:
	CMainMenu* m_pMainMenu;
};

class CExitActionSignal : public vgui::ActionSignal
{
public:
	CExitActionSignal( CMainMenu* pMainMenu )
		: m_pMainMenu( pMainMenu )
	{
	}

	void actionPerformed( vgui::Panel* panel ) override
	{
		//TODO: open a pop-up menu. - Solokiller

		SDL_Event event;

		event.type = SDL_WINDOWEVENT;
		event.window.event = SDL_WINDOWEVENT_CLOSE;
		event.window.windowID = SDL_GetWindowID( g_Video.GetWindow() );

		SDL_PushEvent( &event );
	}

private:
	CMainMenu* m_pMainMenu;
};

CMainMenu::CMainMenu( vgui::Panel* pRoot, int x, int y, int wide, int tall )
	: vgui::Panel( x, y, wide, tall )
{
	setParent( pRoot );

	CreateBackground();

	m_pOptions = new vgui::Button( "Options", static_cast<int>( wide * 0.05 ), static_cast<int>( tall * 0.75 ) );

	m_pOptions->setParent( this );

	m_pOptions->setFgColor( 0, 0, 0, 255 );

	m_pOptions->addActionSignal( new COptionsActionSignal( this ) );

	m_pExit = new vgui::Button( "Exit", static_cast<int>( wide * 0.05 ), static_cast<int>( tall * 0.75 ) + m_pOptions->getTall() );

	m_pExit->setParent( this );

	m_pExit->setFgColor( 0, 0, 0, 255 );

	m_pExit->addActionSignal( new CExitActionSignal( this ) );
}

CMainMenu::~CMainMenu()
{
}

COptionsDialog* CMainMenu::CreateOptionsDialog()
{
	if( !m_pOptionsDialog )
		m_pOptionsDialog = new COptionsDialog( g_Engine.GetRootPanel(), 0, 0, 400, 300 );

	return m_pOptionsDialog;
}

void CMainMenu::CreateBackground()
{
	auto pBackground = new vgui::Panel();

	pBackground->setParent( this );

	int wide, tall;

	getSize( wide, tall );

	pBackground->setSize( wide, tall );

	const float flXScale = g_Video.GetWidth() / 800.0f;
	const float flYScale = g_Video.GetHeight() / 600.0f;
	const int iXOffsetScale = static_cast<int>( ceil( 256 * flXScale ) );
	const int iYOffsetScale = static_cast<int>( ceil( 256 * flYScale ) );

	char szFileName[ MAX_PATH ];

	for( size_t uiIndex = 0; uiIndex < 4 * 3; ++uiIndex )
	{
		snprintf( szFileName, sizeof( szFileName ), "resource/background/800_%u_%c_loading.tga", ( uiIndex / 4 ) + 1, 'a' + ( uiIndex % 4 ) );

		auto pImage = static_cast<vgui::RDBitmapTGA*>( vgui_LoadTGA( szFileName, true, true ) );

		//Resize the images so they fit the default resolution better. The resolution scaling will take care of the rest. - Solokiller
		pImage->SetXScale( 640 / 800.0f );
		pImage->SetYScale( 480 / 600.0f );

		auto pImagePanel = new vgui::ImagePanel( pImage );

		pImagePanel->setParent( pBackground );

		pImagePanel->setPos( iXOffsetScale * ( uiIndex % 4 ), iYOffsetScale * ( uiIndex / 4 ) );
	}
}
