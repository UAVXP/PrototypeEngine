#ifndef ADDONINSTALLER_CADDONINSTALLER_H
#define ADDONINSTALLER_CADDONINSTALLER_H

#include "steam_api.h"

#include "Platform.h"

#include "IMetaTool.h"

#include "lib/CLibrary.h"

class IFileSystem2;

class CAddonInstaller final : public IMetaTool
{
public:
	CAddonInstaller() = default;

	IMetaLoader* GetLoader() { return m_pLoader; }

	const char* GetGameDirectory() const { return m_szGameDir; }

	const char* GetWorkingDirectory() const { return m_szWorkingDir; }

	CSteamAPIContext& GetSteamAPIContext() { return m_SteamAPIContext; }

	bool Startup( IMetaLoader& loader, CreateInterfaceFn* pFactories, const size_t uiNumFactories ) override;

	bool Run() override;

	void Shutdown() override;

private:
	IMetaLoader* m_pLoader = nullptr;

	char m_szGameDir[ MAX_PATH ] = {};

	char m_szWorkingDir[ MAX_PATH ] = {};

	CLibrary m_steam_api;

	bool m_bSteamAPIInitialized = false;

	CSteamAPIContext m_SteamAPIContext;

	IFileSystem2* m_pFileSystem = nullptr;
};

#endif //ADDONINSTALLER_CADDONINSTALLER_H
