#include <cstring>

#include <VGUI_ToggleButton.h>

#include "VGUI_TabPanel2.h"

namespace vgui
{
//Note: TabPanel inserts tabs in the front of the list, so all index lookups have to be reversed in order to return a logical tab. - Solokiller

int TabPanel2::GetTabCount() const
{
	return this->_tabArea->getChildCount();
}

ToggleButton* TabPanel2::GetButtonByIndex( int index )
{
	return static_cast<ToggleButton*>( this->_tabArea->getChild( GetTabCount() - index - 1 ) );
}

Panel* TabPanel2::GetTabByIndex( int index )
{
	return this->_clientArea->getChild( GetTabCount() - index - 1 );
}

int TabPanel2::GetIndexByText( const char* const pszText ) const
{
	if( !pszText )
		return INVALID_TAB_INDEX;

	const int iTabCount = GetTabCount();

	for( int iIndex = 0; iIndex < iTabCount; ++iIndex )
	{
		auto pButton = static_cast<ToggleButton*>( this->_tabArea->getChild( iIndex ) );

		if( strcmp( pButton->GetText(), pszText ) == 0 )
		{
			return iIndex;
		}
	}

	return INVALID_TAB_INDEX;
}

ToggleButton* TabPanel2::GetButtonByText( const char* const pszText )
{
	const auto index = GetIndexByText( pszText );

	if( index == INVALID_TAB_INDEX )
		return nullptr;

	return static_cast<ToggleButton*>( this->_tabArea->getChild( GetTabCount() - index - 1 ) );
}

Panel* TabPanel2::GetTabByText( const char* const pszText )
{
	const auto index = GetIndexByText( pszText );

	if( index == INVALID_TAB_INDEX )
		return nullptr;

	return this->_clientArea->getChild( GetTabCount() - index - 1 );
}
}