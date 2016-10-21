#include <VGUI_ActionSignal.h>
#include <VGUI_Button.h>
#include <VGUI_ListPanel.h>

#include "Engine.h"
#include "FileSystem2.h"
#include "Logging.h"

#include "CCreateServerDialog.h"

class CSelectMapActionSignal : public vgui::ActionSignal
{
public:
	CSelectMapActionSignal( CCreateServerDialog* pDialog )
		: m_pDialog( pDialog )
	{
	}

	void actionPerformed( vgui::Panel* panel ) override
	{
		m_pDialog->MapSelected( panel );
	}

private:
	CCreateServerDialog* m_pDialog;
};

class CStartServerActionSignal : public vgui::ActionSignal
{
public:
	CStartServerActionSignal( CCreateServerDialog* pDialog )
		: m_pDialog( pDialog )
	{
	}

	void actionPerformed( vgui::Panel* panel ) override
	{
		m_pDialog->StartServer();
	}

private:
	CCreateServerDialog* m_pDialog;
};

CCreateServerDialog::CCreateServerDialog( vgui::Panel* pParent, int x, int y, int wide, int tall )
	: vgui::Dialog( x, y, wide, tall )
{
	setParent( pParent );

	m_pMapList = new vgui::ListPanel( 0, 0, wide / 2, tall / 2 );

	m_pMapList->setParent( this );

	m_pSignal = new CSelectMapActionSignal( this );

	m_pStartServer = new vgui::Button( "Start Server", wide / 2, 0 );

	m_pStartServer->setParent( this );

	m_pStartServer->setFgColor( 0, 0, 0, 255 );

	m_pStartServer->addActionSignal( new CStartServerActionSignal( this ) );

	PopulateMapList();
}

void CCreateServerDialog::MapSelected( vgui::Panel* pPanel )
{
	m_pMap = static_cast<vgui::Button*>( pPanel );
}

void CCreateServerDialog::StartServer()
{
	if( m_pMap )
	{
		Msg( "Starting map %s\n", m_pMap->GetText() );

		setVisible( false );

		g_Engine.GetMainMenu()->setVisible( false );
	}
}

void CCreateServerDialog::PopulateMapList()
{
	FileFindHandle_t find;

	if( auto pszFileName = g_pFileSystem->FindFirstEx( "maps/*.bsp", &find, FileSystemFindFlag::SKIP_IDENTICAL_PATHS ) )
	{
		do
		{
			auto pButton = new vgui::Button( pszFileName, 0, 0, 200, 20 );

			pButton->setFgColor( 0, 0, 0, 255 );

			pButton->addActionSignal( m_pSignal );

			m_pMapList->addItem( pButton );
		}
		while( pszFileName = g_pFileSystem->FindNext( find ) );

		g_pFileSystem->FindClose( find );
	}
}
