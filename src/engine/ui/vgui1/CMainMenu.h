#ifndef ENGINE_UI_VGUI1_CMAINMENU_H
#define ENGINE_UI_VGUI1_CMAINMENU_H

#include <VGUI_Panel.h>

class CMainMenu final : public vgui::Panel
{
public:
	CMainMenu( vgui::Panel* pRoot, int x, int y, int wide, int tall );
	virtual ~CMainMenu();

private:
	void CreateBackground();
};

#endif //ENGINE_UI_VGUI1_CMAINMENU_H
