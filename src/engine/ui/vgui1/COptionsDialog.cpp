#include <VGUI_ToggleButton.h>

#include "COptionsDialog.h"

COptionsDialog::COptionsDialog( vgui::Panel* pParent, int x, int y, int wide, int tall )
	: vgui::Dialog( x, y, wide, tall )
{
	m_pTabs = new vgui::TabPanel2( x, y, wide, tall );
	m_pTabs->setParent( this );

	auto pTab = m_pTabs->addTab( "Multiplayer" );

	m_pTabs->addTab( "Audio" );

	for( int iIndex = 0; iIndex < m_pTabs->GetTabCount(); ++iIndex )
	{
		auto pButton = m_pTabs->GetButtonByIndex( iIndex );

		pButton->setFgColor( 0, 0, 0, 255 );
	}

	setParent( pParent );
}

COptionsDialog::~COptionsDialog()
{
	delete m_pTabs;
}
