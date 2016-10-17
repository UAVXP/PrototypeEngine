#ifndef ENGINE_UI_VGUI1_COPTIONSDIALOG_H
#define ENGINE_UI_VGUI1_COPTIONSDIALOG_H

#include "VGUI1/VGUI_Dialog.h"
#include "VGUI1/VGUI_TabPanel2.h"

class COptionsDialog : public vgui::Dialog
{
public:
	COptionsDialog( vgui::Panel* pParent, int x, int y, int wide, int tall );
	~COptionsDialog();

private:
	vgui::TabPanel2* m_pTabs = nullptr;
};

#endif //ENGINE_UI_VGUI1_COPTIONSDIALOG_H
