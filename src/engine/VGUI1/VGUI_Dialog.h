#ifndef ENGINE_VGUI1_VGUI_DIALOG_H
#define ENGINE_VGUI1_VGUI_DIALOG_H

#include <VGUI_Panel.h>

namespace vgui
{
/**
*	A dialog that can be centered on a particular panel or region.
*/
class Dialog : public Panel
{
public:
	using Panel::Panel;

	/**
	*	Centers the panel in the middle of the screen.
	*/
	void CenterOnScreen();

	/**
	*	Centers on a panel, or the screen if pPanel is null.
	*/
	void CenterOnPanel( Panel* pPanel );

	/**
	*	Centers on this panel's parent, or the screen if it has no parent.
	*/
	void CenterOnParent();

	/**
	*	Centers on the given absolute coordinates.
	*/
	void CenterOnPoint( int x, int y );

	/**
	*	Centers in the given region.
	*/
	void CenterInRegion( int wide, int tall );

	/**
	*	Centers in the given region.
	*/
	void CenterInRegion( int x, int y, int wide, int tall );
};
}

#endif //ENGINE_VGUI1_VGUI_DIALOG_H
