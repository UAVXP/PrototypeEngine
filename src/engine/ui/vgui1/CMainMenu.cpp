#include <VGUI_ImagePanel.h>

#include "VGUI1/VGUI_RDBitmapTGA.h"
#include "VGUI1/vgui_loadtga.h"

#include "Engine.h"

#include "CMainMenu.h"

CMainMenu::CMainMenu( vgui::Panel* pRoot, int x, int y, int wide, int tall )
	: vgui::Panel( x, y, wide, tall )
{
	setParent( pRoot );

	CreateBackground();
}

CMainMenu::~CMainMenu()
{
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
