#include <VGUI_ActionSignal.h>
#include <VGUI_Button.h>
#include <VGUI_ListPanel.h>

#include "Common.h"

#include "Engine.h"
#include "FileSystem2.h"
#include "Logging.h"
#include "StringUtils.h"

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

		g_MapManager.LoadMap( m_pMap->GetText() );
	}
}

void CCreateServerDialog::PopulateMapList()
{
	FileFindHandle_t find;

	if( auto pszFileName = g_pFileSystem->FindFirstEx( "maps/*.bsp", &find, FileSystemFindFlag::SKIP_IDENTICAL_PATHS ) )
	{
		char szFileName[ MAX_PATH ];
		char szMapName[ MAX_PATH ];

		do
		{
			UTIL_SafeStrncpy( szFileName, pszFileName, sizeof( szFileName ) );

			UTIL_FixSlashes( szFileName );

			const int iResult = sscanf( szFileName, "maps/%s.bsp", szMapName );

			if( iResult == 1 )
			{
				szMapName[ sizeof( szMapName ) - 1 ] = '\0';

				const size_t uiLength = strlen( szMapName );

				const size_t uiExtLength = strlen( BSP_FILE_EXT );

				//Trim the .bsp part.
				if( uiLength > uiExtLength )
				{
					szMapName[ uiLength - uiExtLength ] = '\0';
				}

				auto pButton = new vgui::Button( szMapName, 0, 0, 200, 20 );

				pButton->setFgColor( 0, 0, 0, 255 );

				pButton->addActionSignal( m_pSignal );

				m_pMapList->addItem( pButton );
			}
			else
				Msg( "Failed to extract map name from filename\n" );
		}
		while( pszFileName = g_pFileSystem->FindNext( find ) );

		g_pFileSystem->FindClose( find );
	}
}
