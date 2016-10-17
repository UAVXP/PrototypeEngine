#ifndef ENGINE_VGUI1_TABPANEL2_H
#define ENGINE_VGUI1_TABPANEL2_H

#include <VGUI_TabPanel.h>

namespace vgui
{
class ToggleButton;

/**
*	Class to add missing functionality to TabPanel.
*	@see TabPanel
*/
class TabPanel2 : public TabPanel
{
public:
	static const int INVALID_TAB_INDEX = -1;

public:
	using TabPanel::TabPanel;

	/**
	*	@return The number of tabs.
	*/
	int GetTabCount() const;

	/**
	*	@return The button belonging to the tab at the given index.
	*/
	ToggleButton* GetButtonByIndex( int index );

	/**
	*	@return The tab at the given index.
	*/
	Panel* GetTabByIndex( int index );

	/**
	*	Gets the index of a tab by the text given to addTab.
	*	Returns INVALID_TAB_INDEX if the tab couldn't be found.
	*/
	int GetIndexByText( const char* const pszText ) const;

	/**
	*	Gets the button of a tab by the text given to addTab.
	*	Returns nullptr if the tab couldn't be found.
	*/
	ToggleButton* GetButtonByText( const char* const pszText );

	/**
	*	Gets a tab by the text given to addTab.
	*	Returns nullptr if the tab couldn't be found.
	*/
	Panel* GetTabByText( const char* const pszText );
};
}

#endif //ENGINE_VGUI1_TABPANEL2_H
