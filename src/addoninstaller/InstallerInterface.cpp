#include "Platform.h"
#include "FilePaths.h"
#include "AddonInstaller.h"

#include "InstallerInterface.h"

#ifdef WIN32
//See post VS 2015 update 3 delayimp.h for the reason why this has to be defined. - Solokiller
#define DELAYIMP_INSECURE_WRITABLE_HOOKS
#include <delayimp.h>

FARPROC WINAPI DelayHook(
	unsigned        dliNotify,
	PDelayLoadInfo  pdli
)
{
	if( dliNotify == dliNotePreLoadLibrary )
	{
		if( strcmp( pdli->szDll, "Tier1.dll" ) == 0 )
		{
			char szPath[ MAX_PATH ];

			if( !( *g_AddonInstaller.GetGameDirectory() ) )
				return nullptr;

			const int iResult = snprintf( szPath, sizeof( szPath ), "%s/%s/%s", g_AddonInstaller.GetGameDirectory(), filepaths::BIN_DIR, pdli->szDll );

			if( iResult < 0 || static_cast<size_t>( iResult ) >= sizeof( szPath ) )
				return nullptr;

			HMODULE hLib = LoadLibraryA( szPath );

			return ( FARPROC ) hLib;
		}
	}

	return nullptr;
}

ExternC PfnDliHook __pfnDliNotifyHook2 = DelayHook;

ExternC PfnDliHook   __pfnDliFailureHook2 = nullptr;
#endif
