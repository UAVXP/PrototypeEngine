#include <cstdlib>
#include <algorithm>

#include "steam/steam_api.h"

#include "CAddonDataLoader.h"

#include "SteamHelpers.h"
#include "Helpers.h"

static void LogHelp( const CAppInfo& appInfo )
{
	Log( LogLevel::ALWAYS, "Could not find game content for %s\n", appInfo.szName.c_str() );
	Log( LogLevel::ALWAYS, "Please verify that the game is installed, and that it has been run at least once\n" );
	Log( LogLevel::ALWAYS, "Verify that the path is correct\n" );
	Log( LogLevel::ALWAYS, "Verify the game's files through Steam\nRight-click the game->Properties->Local Files->Verify integrity of game cache\n" );
	Log( LogLevel::ALWAYS, "Verify that Sven Co-op's files are properly installed using the same method\n" );
	Log( LogLevel::ALWAYS, "Ensure that this program is being run from the Sven Co-op/svencoop/ directory\n" );
}

static int RealMain( const char* const pszCommandLine, HINSTANCE hInstance, int nCmdShow )
{
	const char* pszValue = nullptr;

	bool bSilent = false;

	if( ParseCommandLine( pszCommandLine, "-loglevel", &pszValue ) )
	{
		int iLogLevel = atoi( pszValue );

		iLogLevel = std::min( static_cast<int>( LogLevel::LAST ), std::max( static_cast<int>( LogLevel::FIRST ), iLogLevel ) );

		g_LogLevel = static_cast<LogLevel>( iLogLevel );
	}

	/*
	Disabled until all code handles silent properly
	if( ParseCommandLine( pszCommandLine, "-silent" ) )
	{
		bSilent = true;
		g_LogLevel = LogLevel::SILENT;
	}
	*/

	Log( LogLevel::EXTRA, "Command line: %s\n\n", pszCommandLine );

	if( SetCWD( pszCommandLine ) )
	{
		Log( LogLevel::ALWAYS, "\n" );

		if( LoadFilesystem() )
		{
			//TODO: move to file
			std::vector<std::string> op4Maps =
			{
				"of0a0",
				"of1a1",
				"of1a2",
				"of1a3",
				"of1a4",
				"of1a4b",
				"of1a5",
				"of1a5b",
				"of1a6",
				"of2a1",
				"of2a1b",
				"of2a4",
				"of2a5",
				"of2a6",
				"of3a1",
				"of3a2",
				"of3a4",
				"of3a5",
				"of3a6",
				"of4a1",
				"of4a2",
				"of4a3",
				"of4a4",
				"of4a5",
				"of5a1",
				"of5a2",
				"of5a3",
				"of5a4",
				"of6a1",
				"of6a2",
				"of6a3",
				"of6a4",
				"of6a4b",
				"of6a5"
			};

			std::vector<std::string> bshiftMaps = 
			{
				"ba_canal1",
				"ba_canal1b",
				"ba_canal2",
				"ba_canal3",
				"ba_elevator",
				"ba_maint",
				"ba_outro",
				"ba_power1",
				"ba_power2",
				"ba_security1",
				"ba_security2",
				"ba_teleport1",
				"ba_teleport2",
				"ba_tram1",
				"ba_tram2",
				"ba_tram3",
				"ba_xen1",
				"ba_xen2",
				"ba_xen3",
				"ba_xen4",
				"ba_xen5",
				"ba_xen6",
				"ba_yard1",
				"ba_yard2",
				"ba_yard3",
				"ba_yard3a",
				"ba_yard3b",
				"ba_yard4",
				"ba_yard4a",
				"ba_yard5",
				"ba_yard5a"
			};

			std::vector<CAppInfo> apps =
			{
				{ 50,	"Opposing Force",	"gearbox",	"opfor_support.sven",	false,	std::move( op4Maps ) },
				{ 130,	"Blue Shift",		"bshift",	"bshift_support.sven",	true,	std::move( bshiftMaps ) }
			};

			{
				CAddonDataLoader loader;

				if( loader.Load( "addons.txt" ) )
				{
					Log( LogLevel::NORMAL, "Loaded addon settings from file\n\n" );

					apps = std::move( loader.GetAppInfos() );
				}
			}

			CSteamAPI steamAPI;

			if( steamAPI.IsInitialized() )
			{
				if( !GetDirectoriesFromSteam( apps ) )
					Log( LogLevel::ALWAYS, "Failed to get installation directories from Steam\n" );
			}
			else
				Log( LogLevel::ALWAYS, "Steam not running or not installed\n\n" );

			if( AskForDirectories( apps ) )
			{
				if( HasRequiredFiles() )
				{
					Log( LogLevel::NORMAL, "\nAll required installer files found\n\n" );

					for( const auto& appInfo : apps )
					{
						if( appInfo.szPath[ 0 ] )
						{
							if( !CopyGameFiles( appInfo ) )
								LogHelp( appInfo );
						}
					}

					Log( LogLevel::NORMAL, "\nDone!\n" );
				}
				else
				{
					Log( LogLevel::ALWAYS, "Unable to find some requires files\n" );
				}
			}
			else
				Log( LogLevel::ALWAYS, "Unable to find app install directories\n" );
		}
		else
			Log( LogLevel::ALWAYS, "Failed to load file system\n" );
	}

	if( !bSilent )
	{
		Log( LogLevel::ALWAYS, "Press Enter to continue..." );
		getchar();
	}

	// exit
	return EXIT_SUCCESS;
}

#ifdef WIN32
void MiniDumpFunction( unsigned int nExceptionCode, EXCEPTION_POINTERS *pException )
{
	// You can build and set an arbitrary comment to embed in the minidump here,
	// maybe you want to put what level the user was playing, how many players on the server,
	// how much memory is free, etc...
	SteamAPI_SetMiniDumpComment( "AddonInstaller: This shouldn't happen. Let us know at www.svencoop.com\n" );

	// The 0 here is a build ID, we don't set it
	SteamAPI_WriteMiniDump( nExceptionCode, pException, 0 );
}

#include<delayimp.h>

FARPROC WINAPI delayHook( unsigned dliNotify, PDelayLoadInfo pdli )
{
	if( dliNotify == dliNotePreLoadLibrary )
	{
		if( strcmp( pdli->szDll, "steam_api.dll" ) == 0 )
		{
			//TODO: update the engine so it uses Steamworks v135a or later, then use the api dll provided by it
			return ( FARPROC ) LoadLibrary( "../steam_api.dll" );
		}
	}

	return nullptr;
}
PfnDliHook __pfnDliNotifyHook2 = delayHook;
#endif //WIN32

#if !defined( WIN32 ) || defined( _CONSOLE )
int main( int iArgc, char** ppszArgv )
#else
int APIENTRY WinMain( HINSTANCE hInstance,
					  HINSTANCE hPrevInstance,
					  LPSTR     lpCmdLine,
					  int       nCmdShow )
#endif
{
#if !defined( WIN32 ) || defined( _CONSOLE )
	char szCmdLine[ 1024 ];
	char *pszStart = szCmdLine;
	char * const pszEnd = szCmdLine + V_ARRAYSIZE( szCmdLine );

	*szCmdLine = '\0';

	for( int i = 0; i < iArgc; i++ )
	{
		const char *parm = ppszArgv[ i ];

		const bool bQuoted = strstr( parm, " " ) != nullptr;

		if( bQuoted )
			*pszStart++ = '\"';

		while( *parm && ( pszStart < pszEnd ) )
		{
			*pszStart++ = *parm++;
		}

		if( ( pszStart < pszEnd ) && bQuoted )
			*pszStart++ = '\"';

		if( pszStart >= pszEnd )
			break;

		if( i < iArgc - 1 )
			*pszStart++ = ' ';
	}

	if( pszStart < pszEnd )
		*pszStart = '\0';
	else
		szCmdLine[ V_ARRAYSIZE( szCmdLine ) - 1 ] = '\0';
#else
	char* szCmdLine = lpCmdLine;
#endif

#ifndef WIN32
	return RealMain( szCmdLine, 0, 0 );
#else //WIN32

	// All we do here is call the real main function after setting up our se translator
	// this allows us to catch exceptions and report errors to Steam.
	//
	// Note that you must set your compiler flags correctly to enable structured exception 
	// handling in order for this particular setup method to work.
	if( IsDebuggerPresent() )
	{
		// We don't want to mask exceptions (or report them to Steam!) when debugging.
		// If you would like to step through the exception handler, attach a debugger
		// after running the game outside of the debugger.
		return RealMain( szCmdLine, 0, 0 );
	}

	_set_se_translator( MiniDumpFunction );

	try  // this try block allows the SE translator to work
	{
		return RealMain( szCmdLine, 0, 0 );
	}
	catch( ... )
	{
		return -1;
	}
#endif //WIN32
}