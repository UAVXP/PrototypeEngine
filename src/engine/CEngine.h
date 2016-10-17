#ifndef ENGINE_CENGINE_H
#define ENGINE_CENGINE_H

#include <memory>

#include "Platform.h"

#include "lib/CLibrary.h"

#include "ui/vgui1/CMainMenu.h"

#include "IMetaTool.h"

namespace vgui
{
class Panel;
}

class CSchemeManager;

class CEngine final : public IMetaTool
{
public:
	CEngine() = default;

	/**
	*	Gets the game directory that the engine mod is in.
	*	This is essentially the working directory for the game directory.
	*/
	const char* GetMyGameDir() const { return m_szMyGameDir; }

	IMetaLoader* GetLoader() { return m_pLoader; }

	CSchemeManager* GetSchemeManager() { return m_pSchemeManager; }

	vgui::Panel* GetRootPanel() { return m_pRootPanel; }

	void SetMyGameDir( const char* const pszGameDir );

	bool Startup( IMetaLoader& loader, CreateInterfaceFn* pFactories, const size_t uiNumFactories ) override;

	bool Run() override;

	void Shutdown() override;

	void RunFrame();

private:
	bool SetupFileSystem();

	bool HostInit();

	void CreateMainMenu();

	void RenderFrame();

	void RenderVGUI1();

private:
	char m_szMyGameDir[ MAX_PATH ] = {};

	IMetaLoader* m_pLoader = nullptr;

	CLibrary m_steam_api;

	bool m_bSteamAPIInitialized = false;

	CSchemeManager* m_pSchemeManager = nullptr;

	vgui::Panel* m_pRootPanel = nullptr;

	std::unique_ptr<CMainMenu> m_MainMenu;

private:
	CEngine( const CEngine& ) = delete;
	CEngine& operator=( const CEngine& ) = delete;
};

#endif //ENGINE_CENGINE_H
