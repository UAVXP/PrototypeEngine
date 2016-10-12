#include "steam/SteamWrapper.h"

#include "AddonInstaller.h"
#include "FilePaths.h"
#include "FileSystem2.h"
#include "Logging.h"
#include "IMetaLoader.h"

#include "CAddonInstaller.h"

EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CAddonInstaller, IMetaTool, DEFAULT_IMETATOOL_NAME, g_AddonInstaller );

bool CAddonInstaller::Startup( IMetaLoader& loader, CreateInterfaceFn* pFactories, const size_t uiNumFactories )
{
	m_pLoader = &loader;

	if( !m_pLoader->GetGameDirectory( m_szGameDir, sizeof( m_szGameDir ) ) )
		return false;

	if( !( *m_szGameDir ) )
	{
		UTIL_ShowMessageBox( "No game directory set", "Error", LogType::ERROR );
		return false;
	}

	if( !m_pLoader->GetToolDirectory( m_szWorkingDir, sizeof( m_szWorkingDir ) ) )
		return false;

	if( !( *m_szWorkingDir ) )
	{
		UTIL_ShowMessageBox( "No working directory set", "Error", LogType::ERROR );
		return false;
	}

	m_steam_api = Steam_LoadSteamAPI( filepaths::BIN_DIR );

	m_bSteamAPIInitialized = Steam_InitWrappers( m_steam_api, true );

	if( !m_bSteamAPIInitialized )
	{
		return false;
	}

	if( !m_SteamAPIContext.Init() )
	{
		UTIL_ShowMessageBox( "Failed to initialize Steam API Context. Exiting...\n", "Fatal Error", LogType::ERROR );
		return false;
	}

	for( size_t uiIndex = 0; uiIndex < uiNumFactories; ++uiIndex )
	{
		auto factory = pFactories[ uiIndex ];

		if( !m_pFileSystem )
		{
			m_pFileSystem = static_cast<IFileSystem2*>( factory( FILESYSTEM2_INTERFACE_VERSION, nullptr ) );
		}
	}

	if( !m_pFileSystem )
	{
		Msg( "Couldn't instantiate the filesystem\n" );
		return false;
	}

	return true;
}

bool CAddonInstaller::Run()
{
	return true;
}

void CAddonInstaller::Shutdown()
{
	if( m_steam_api.IsLoaded() )
	{
		if( m_bSteamAPIInitialized )
			SteamAPI_Shutdown();

		m_steam_api.Free();
	}
}