//TODO: make one header that includes the platform headers - Solokiller
#include "Platform.h"

#ifdef WIN32
#include "Shellapi.h"
#include <codecvt>
#endif

#ifdef LINUX
#include <fstream>
#include <vector>
#endif

namespace
{
static const int ARGC_UNINITIALIZED = -1;
}

namespace engine
{
int argc = ARGC_UNINITIALIZED;
char** argv = nullptr;
}

#ifdef _WIN32

// Required DLL entry point
BOOL WINAPI DllMain(
	HINSTANCE hinstDLL,
	DWORD fdwReason,
	LPVOID lpvReserved )
{
	if( fdwReason == DLL_PROCESS_ATTACH )
	{
	}
	else if( fdwReason == DLL_PROCESS_DETACH )
	{
	}
	return TRUE;
}

namespace engine
{
bool InitCommandLine()
{
	//If it was already processed once, return cached values without reprocessing. - Solokiller
	if( argc == ARGC_UNINITIALIZED )
	{
		const wchar_t* pszCommandLine = GetCommandLineW();

		if( !pszCommandLine )
			return false;

		int iArgC;

		wchar_t** ppszArgV = CommandLineToArgvW( pszCommandLine, &iArgC );

		if( !ppszArgV )
			return false;

		argc = iArgC;

		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

		argv = new char*[ argc ];

		for( int iArg = 0; iArg < argc; ++iArg )
		{
			const std::string szArg = converter.to_bytes( ppszArgV[ iArg ] );

			argv[ iArg ] = new char[ szArg.length() + 1 ];

			strcpy( argv[ iArg ], szArg.c_str() );
		}

		LocalFree( ppszArgV );
	}

	return true;
}
}
#endif

#ifdef LINUX
namespace engine
{
bool InitCommandLine()
{
	//If it was already processed once, return cached values without reprocessing. - Solokiller
	if( argc == ARGC_UNINITIALIZED )
	{
		//Read the command line from this special file.
		std::ifstream file( "/proc/self/cmdline", std::ios::in );

		if( !file.is_open() )
			return false;

		std::string szLine;

		std::vector<std::string> vecArgs;

		//Command line is a series of null terminated strings.
		while( std::getline( file, szLine, '\0' ) )
		{
			vecArgs.emplace_back( std::move( szLine ) );
		}

		//Convert to argv vector.
		argc = vecArgs.size();

		argv = new char*[ argc ];

		int iArg = 0;

		for( const auto& szArg : vecArgs )
		{
			argv[ iArg ] = new char[ szArg.length() ];

			strcpy( argv[ iArg ], szArg.c_str() );

			++iArg;
		}
	}

	return true;
}
}
#endif