#ifndef ENGINE_UI_VGUI1_CMAINMENU_H
#define ENGINE_UI_VGUI1_CMAINMENU_H

#include <VGUI_Panel.h>

namespace vgui
{
class Button;
}

class COptionsDialog;
class CCreateServerDialog;

class CMainMenu final : public vgui::Panel
{
public:
	CMainMenu( vgui::Panel* pRoot, int x, int y, int wide, int tall );
	virtual ~CMainMenu();

	COptionsDialog* CreateOptionsDialog();

	CCreateServerDialog* CreateCreateServerDialog();

private:
	void CreateBackground();

private:
	vgui::Button* m_pCreateServer = nullptr;
	vgui::Button* m_pOptions = nullptr;
	vgui::Button* m_pExit = nullptr;

	COptionsDialog* m_pOptionsDialog = nullptr;
	CCreateServerDialog* m_pCreateServerDialog = nullptr;
};

#endif //ENGINE_UI_VGUI1_CMAINMENU_H
