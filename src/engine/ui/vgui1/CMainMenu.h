#ifndef ENGINE_UI_VGUI1_CMAINMENU_H
#define ENGINE_UI_VGUI1_CMAINMENU_H

#include <VGUI_Panel.h>

namespace vgui
{
class Button;
}

class COptionsDialog;

class CMainMenu final : public vgui::Panel
{
public:
	CMainMenu( vgui::Panel* pRoot, int x, int y, int wide, int tall );
	virtual ~CMainMenu();

	COptionsDialog* CreateOptionsDialog();

private:
	void CreateBackground();

private:
	vgui::Button* m_pOptions = nullptr;
	vgui::Button* m_pExit = nullptr;

	COptionsDialog* m_pOptionsDialog = nullptr;
};

#endif //ENGINE_UI_VGUI1_CMAINMENU_H
