#ifndef ENGINE_UI_VGUI1_CCREATESERVERDIALOG_H
#define ENGINE_UI_VGUI1_CCREATESERVERDIALOG_H

#include "VGUI1/VGUI_Dialog.h"

namespace vgui
{
class Button;
class ListPanel;
}

class CSelectMapActionSignal;

class CCreateServerDialog : public vgui::Dialog
{
public:
	CCreateServerDialog( vgui::Panel* pParent, int x, int y, int wide, int tall );

	void MapSelected( vgui::Panel* pPanel );

	void StartServer();

private:
	void PopulateMapList();

private:
	vgui::ListPanel* m_pMapList;
	CSelectMapActionSignal* m_pSignal;
	vgui::Button* m_pMap = nullptr;

	vgui::Button* m_pStartServer;
};

#endif //ENGINE_UI_VGUI1_CCREATESERVERDIALOG_H
