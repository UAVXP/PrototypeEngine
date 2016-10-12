#include <cstdio>
#include <cctype>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdarg>

#include <fstream>
#include <string>

#include "steam_api.h"

#include "Platform.h"

#include "Helpers.h"

#include "AddonInstaller.h"
#include "FileSystem2.h"

#include <cassert>

#undef CopyFile

LogLevel g_LogLevel = LogLevel::NORMAL;

void Log( const LogLevel log, const char* const pszFormat, ... )
{
	if( log > g_LogLevel )
		return;

	va_list list;

	va_start( list, pszFormat );
	vprintf( pszFormat, list );
	va_end( list );

	fflush( stdout );
}

/*
* Helper class to redirect output when executing other programs
*/
class CRedirectOutput final
{
public:
	CRedirectOutput( const std::string& szCommand )
		: m_pFile( nullptr )
	{
		//Also redirect stderr
		const std::string szFullCommand = std::string( "cmd /S /C \"" ) + szCommand + " 2>&1\"";

		m_pFile = popen( szFullCommand.c_str(), "r" );
	}

	~CRedirectOutput()
	{
		Close();
	}

	bool IsOpen() const
	{
		return m_pFile != nullptr;
	}

	void Close()
	{
		if( IsOpen() )
		{
			pclose( m_pFile );
			m_pFile = nullptr;
		}
	}

	void FlushToLog()
	{
		if( !IsOpen() )
			return;

		char szBuffer[ 512 ];

		while( !feof( m_pFile ) )
		{
			if( fgets( szBuffer, sizeof( szBuffer ), m_pFile ) )
			{
				Log( LogLevel::VERBOSE, "%s", szBuffer );
			}
		}
	}

private:
	FILE* m_pFile;

private:
	CRedirectOutput( const CRedirectOutput& ) = delete;
	CRedirectOutput& operator=( const CRedirectOutput& ) = delete;
};

char* FixSlashes( char* pszPath )
{
	if( !pszPath )
		return nullptr;

	for( char* pszPos = pszPath; *pszPos; ++pszPos )
	{
		if( *pszPos == FILESYSTEM_OTHER_PATH_SEPARATOR_CHAR )
			*pszPos = FILESYSTEM_PATH_SEPARATOR_CHAR;
	}

	return pszPath;
}

QuestionAction AskYNQuestion( const char* const pszQuestion )
{
	Log( LogLevel::ALWAYS, "%s\nEnter Y/N: ", pszQuestion );

	QuestionAction action = QuestionAction::ASKAGAIN;

	do
	{
		const char cAnswer = toupper( getchar() );

		//Empty line; don't flush
		if( cAnswer != '\n' )
		{
			//Flush remaining characters
			while( getchar() != '\n' )
			{
			}
		}

		if( cAnswer == 'Y' )
			action = QuestionAction::YES;
		else if( cAnswer == 'N' )
			action = QuestionAction::NO;
		else
		{
			Log( LogLevel::ALWAYS, "Please enter Y or N: " );
		}
	}
	while( action == QuestionAction::ASKAGAIN );

	return action;
}

bool ValidatePath( const char* const pszPath, const bool bIsDirectory, const bool bSilent )
{
	if( !pszPath )
		return false;

	struct stat statResult;

	if( stat( pszPath, &statResult ) == 0 )
	{
		if( !( bIsDirectory ^ ( ( statResult.st_mode & S_IFDIR ) != 0 ) ) )
		{
			return true;
		}
		else if( !bSilent )
			Log( LogLevel::ALWAYS, "Error: That path is %sa directory\n", bIsDirectory ? "not" : "" );
	}
	else if( !bSilent )
		Log( LogLevel::ALWAYS, "Error: That path does not exist\n" );

	return false;
}

bool DoesFileExist( const char* const pszFilename )
{
	if( !pszFilename )
		return false;

	struct stat statResult;

	if( stat( pszFilename, &statResult ) == 0 )
	{
		//Can't be a directory
		if( !( statResult.st_mode & S_IFDIR ) )
		{
			return true;
		}
	}

	return false;
}

bool AskForDirectory( const char* const pszName, char* pszPath, const size_t uiBufferSize )
{
	if( !pszPath || !uiBufferSize )
		return false;

	//in case we exit early.
	memset( pszPath, 0, uiBufferSize );

	if( !pszName )
		return false;

	size_t uiAttempts = 0;

	do
	{
		Log( LogLevel::ALWAYS, "Please enter the path to the %s installation directory: ", pszName );

		if( fgets( pszPath, uiBufferSize, stdin ) )
		{
			size_t uiLength = strlen( pszPath );

			if( uiLength > 0 )
			{
				//Strip newline
				if( pszPath[ uiLength - 1 ] == '\n' )
				{
					--uiLength;
					pszPath[ uiLength ] = '\0';
				}

				if( ValidatePath( pszPath, true ) )
					break;
			}
		}

		//In case the buffer was modified and contains garbage.
		memset( pszPath, 0, uiBufferSize );

		++uiAttempts;
	}
	while( uiAttempts < MAX_QUESTION_ATTEMPTS );

	return uiAttempts < MAX_QUESTION_ATTEMPTS;
}

/*
* Asks for the HL install directory, and validates the existence of the mod directories
*/
bool AskForHLDirectory( std::vector<CAppInfo>& appInfos )
{
	char szBasePath[ MAX_PATH ];

	if( AskForDirectory( "Half-Life", szBasePath, sizeof( szBasePath ) ) )
	{
		for( auto& appInfo : appInfos )
		{
			//Could have been filled in by Steam beforehand
			if( !appInfo.szPath[ 0 ] )
			{
				const int iRet = snprintf( appInfo.szPath, sizeof( appInfo.szPath ), "%s" FILESYSTEM_PATH_SEPARATOR "%s", szBasePath, appInfo.szModDir.c_str() );

				if( iRet < 0 || iRet >= sizeof( appInfo.szPath ) )
				{
					Log( LogLevel::ALWAYS, "Error: Path is invalid or too long!\n" );
					return false;
				}
				else
				{
					if( !ValidatePath( appInfo.szPath, true, true ) )
					{
						//Couldn't find the install dir, might be somewhere else.
						memset( appInfo.szPath, 0, sizeof( appInfo.szPath ) );
					}
				}
			}
		}
	}
	else
		return false;

	return true;
}

/*
* Asks for the directories to all apps that aren't already known.
* Returns true if all questions were answered, false otherwise.
*/
bool AskForDirectories( std::vector<CAppInfo>& appInfos )
{
	//Are all paths already filled in?
	bool bShouldAsk = false;

	for( auto& appInfo : appInfos )
	{
		if( !appInfo.szPath[ 0 ] )
		{
			bShouldAsk = true;
			break;
		}
	}

	//Ask for the HL install dir first.
	if( bShouldAsk )
	{
		if( !AskForHLDirectory( appInfos ) )
		{
			Log( LogLevel::ALWAYS, "Error: Unable to find installation directory. Please try again after verifying the games are installed and have been run at least once\n" );
			return false;
		}
	}

	{
		size_t uiLongest = 0;

		for( const auto& appInfo : appInfos )
		{
			const size_t uiLength = appInfo.szName.length();

			if( uiLength > uiLongest )
				uiLongest = uiLength;
		}

		Log( LogLevel::ALWAYS, "\nPaths for all games:\n" );

		size_t uiFound = 0;

		for( const auto& appInfo : appInfos )
		{
			const char* pszPath;

			if( appInfo.szPath[ 0 ] )
			{
				++uiFound;
				pszPath = appInfo.szPath;
			}
			else
				pszPath = "Not Found";

			//Align paths
			Log( LogLevel::ALWAYS, "%-*s: %s\n", int( uiLongest ), appInfo.szName.c_str(), pszPath );
		}

		Log( LogLevel::ALWAYS, "\n" );

		if( uiFound > 0 )
		{
			if( AskYNQuestion( "\nAre these paths correct?" ) == QuestionAction::YES )
				return true;
		}
		else
			Log( LogLevel::ALWAYS, "No installations could be found. Please enter the paths manually\n" );
	}

	for( auto& appInfo : appInfos )
	{
		if( !AskForDirectory( appInfo.szName.c_str(), appInfo.szPath, sizeof( appInfo.szPath ) ) )
		{
			Log( LogLevel::ALWAYS, "Unable to find installation directory. Please try again after verifying the game is installed and has been run at least once\n" );
		}
	}

	return true;
}

bool GetDirectoriesFromSteam( std::vector<CAppInfo>& appInfos )
{
	ISteamApps* pApps = g_AddonInstaller.GetSteamAPIContext().SteamApps();

	if( !pApps )
		return false;

	Log( LogLevel::EXTRA, "Requesting app installation directories from Steam...\n" );

	for( auto& appInfo : appInfos )
	{
		if( pApps->BIsAppInstalled( appInfo.appId ) )
		{
			char szDirectory[ MAX_PATH ];

			const uint32 uiLength = pApps->GetAppInstallDir( appInfo.appId, szDirectory, sizeof( szDirectory ) );

			snprintf( appInfo.szPath, sizeof( appInfo.szPath ), "%s" FILESYSTEM_PATH_SEPARATOR "%s", szDirectory, appInfo.szModDir.c_str() );
		}
		else
		{
			Log( LogLevel::EXTRA, "The game %s is not installed\n", appInfo.szName.c_str() );
		}
	}

	Log( LogLevel::EXTRA, "Done\n" );

	return true;
}

fs::path RipentFilename()
{
#ifdef WIN32
	return g_AddonInstaller.GetSCGameDir() / "maps" / "Ripent.exe";
#else
	return g_AddonInstaller.GetSCGameDir() / "ripent";
#endif
}

fs::path UnzipFilename()
{
#ifdef WIN32
	return g_AddonInstaller.GetSCGameDir() / "unzip.exe";
#else
	return "unzip";
#endif
}

fs::path BShiftBSPConverterFilename()
{
#ifdef WIN32
	return g_AddonInstaller.GetSCGameDir() / "BShiftBSPConverter.exe";
#else
	return g_AddonInstaller.GetSCGameDir() / "BShiftBSPConverter";
#endif
}

struct RequiredFile_t
{
	const fs::path filename;
	const char* const pszHelpInfo;
};

#define REQ_FILE_VERIFY_FILES "Please verify the game's files or validate the server installation"

/*
* Pretty ugly, but it gets the job done
*/
bool HasRequiredFiles()
{
	const RequiredFile_t pszRequiredFiles[] =
	{
		{ RipentFilename(), REQ_FILE_VERIFY_FILES },
		{ BShiftBSPConverterFilename(), REQ_FILE_VERIFY_FILES },
#ifdef WIN32
		{ UnzipFilename(), REQ_FILE_VERIFY_FILES }
#else
		{ UnzipFilename(), "You can install unzip using 'apt-get install unzip' on Ubuntu and Debian." }	//Can be anywhere
#endif
	};

	bool bFoundAll = true;

	fs::path path;

	for( const auto& file : pszRequiredFiles )
	{
#ifdef WIN32
		if( !fs::exists( file.filename ) )
#else
		if( access( pszFile, X_OK ) != 0 )
#endif
		{
			Log( LogLevel::ALWAYS, "Unable to find installer file %s\n%s\n", file.filename.u8string().c_str(), file.pszHelpInfo );
			bFoundAll = false;
		}
	}

	return bFoundAll;
}

bool CopyFile( const std::string& szFrom, const std::string& szTo )
{
	std::ifstream from( szFrom, std::ios::binary );

	if( !from )
		return false;

	std::ofstream to( szTo, std::ios::binary );

	if( !to )
		return false;

	to << from.rdbuf();

	return true;
}

#define FS_MAPS_DIR "maps" FILESYSTEM_PATH_SEPARATOR
#define FS_SKIES_DIR "gfx" FILESYSTEM_PATH_SEPARATOR "env" FILESYSTEM_PATH_SEPARATOR

#define FS_FROM_PATH "FROM"
#define FS_MAPS_PATH "MAPS"

#define BSP_EXT ".bsp"
#define ENT_EXT ".ent"

void CopyInteractive( const char* const pszName, const std::string& szFrom, const std::string& szTo, size_t& uiFailedCount, size_t& uiSucceededCount )
{
	Log( LogLevel::VERBOSE, "%s... ", pszName ? pszName : szTo.c_str() );
	fflush( stdout );

	if( !CopyFile( szFrom, szTo ) )
	{
		Log( LogLevel::VERBOSE, "- Failed to copy\n" );
		++uiFailedCount;
	}
	else
	{
		Log( LogLevel::VERBOSE, "Done\n" );
		++uiSucceededCount;
	}
}

/*
* This abuses Valve's filesystem to copy maps.
* Better than writing wrappers ourselves.
*/
bool CopyGameFiles( const CAppInfo& appInfo )
{
	if( !appInfo.szPath[ 0 ] )
		return false;

	if( appInfo.mapNames.empty() )
	{
		Log( LogLevel::ALWAYS, "No maps specified for game %s\n", appInfo.szName.c_str() );
		return false;
	}

	const std::string szBasePath = std::string( appInfo.szPath ) + FILESYSTEM_PATH_SEPARATOR;

	const fs::path destPath = g_AddonInstaller.GetSCGameDir();

	size_t uiFailedCount = 0;

	size_t uiTotalSucceededCount = 0;

	const std::string szMapsPath = szBasePath + FS_MAPS_DIR;

	auto pFileSystem = g_AddonInstaller.GetFileSystem();

	if( !DoesFileExist( ( szMapsPath + appInfo.mapNames[ 0 ] + BSP_EXT ).c_str() ) )
	{
		Log( LogLevel::ALWAYS, "Unable to find required %s content\n", appInfo.szName.c_str() );
		return false;
	}

	Log( LogLevel::NORMAL, "%s: Copying required files to Sven Co-op\n", appInfo.szName.c_str() );

	{
		Log( LogLevel::EXTRA, "Copying maps...\n" );

		size_t uiSucceededCount = 0;

		const fs::path toMapsPath = destPath / FS_MAPS_DIR;

		fs::create_directories( toMapsPath );

		for( const auto& map : appInfo.mapNames )
		{
			CopyInteractive( map.c_str(), szMapsPath + map + BSP_EXT, ( toMapsPath / ( map + BSP_EXT ) ).u8string(), uiFailedCount, uiSucceededCount );
		}

		Log( LogLevel::EXTRA, "Done\nCopied %u maps\n", uiSucceededCount );

		uiTotalSucceededCount += uiSucceededCount;
	}

	{
		Log( LogLevel::EXTRA, "Copying skies...\n" );

		size_t uiSucceededCount = 0;

		const std::string szSkiesPath = szBasePath + FS_SKIES_DIR;

		const fs::path toSkiesPath = destPath / FS_SKIES_DIR;

		fs::create_directories( toSkiesPath );

		pFileSystem->AddSearchPathNoWrite( szSkiesPath.c_str(), FS_FROM_PATH );

		FileFindHandle_t hFile = FILESYSTEM_INVALID_FIND_HANDLE;

		if( const char* pszFilename = pFileSystem->FindFirst( "*", &hFile, FS_FROM_PATH ) )
		{
			do
			{
				if( !pFileSystem->FindIsDirectory( hFile ) )
				{
					CopyInteractive( pszFilename, szSkiesPath + pszFilename, ( toSkiesPath / pszFilename ).u8string(), uiFailedCount, uiSucceededCount );
				}

				pszFilename = pFileSystem->FindNext( hFile );
			}
			while( pszFilename != nullptr );

			pFileSystem->FindClose( hFile );

			Log( LogLevel::EXTRA, "Done\nCopied %u skies\n", uiSucceededCount );

			uiTotalSucceededCount += uiSucceededCount;
		}
		else
			Log( LogLevel::ALWAYS, "Warning: No skies found\n" );

		//Note: if this is not done, it will reuse the path later on!
		pFileSystem->RemoveAllSearchPaths();
	}

	if( uiTotalSucceededCount > 0 )
	{
		Log( LogLevel::NORMAL, "Finished copying files\n" );

		if( uiFailedCount > 0 )
			Log( LogLevel::ALWAYS, "Failed to copy %u files\n", uiFailedCount );
	}
	else
	{
		Log( LogLevel::ALWAYS, "Failed to copy any files; verify that the path is correct and try again\n" );
		return false;
	}

	Log( LogLevel::NORMAL, "Preparing...\n" );

	{
		const std::string szUnzipCommand = 
			std::string( "\"" ) + UnzipFilename().u8string() + "\" -o \"" + 
			destPath.u8string() + FILESYSTEM_PATH_SEPARATOR + appInfo.szSvenSupportFile + "\" -d \"" + ( destPath / "maps" ).u8string() + "\"";

		Log( LogLevel::VERBOSE, "Command: %s\n", szUnzipCommand.c_str() );
		CRedirectOutput redirect( szUnzipCommand.c_str() );

		redirect.FlushToLog();
	}

	if( appInfo.bIsBShiftMapFormat )
	{
		Log( LogLevel::NORMAL, "Converting BSP format...\n" );

		for( const auto& map : appInfo.mapNames )
		{
			const std::string szConvertCommand = 
				std::string( "\"" ) + BShiftBSPConverterFilename().u8string() + "\" \"" + 
				destPath.u8string() + FILESYSTEM_PATH_SEPARATOR + FS_MAPS_DIR + map + BSP_EXT + "\"";

			Log( LogLevel::VERBOSE, "Command: %s\n", szConvertCommand.c_str() );
			CRedirectOutput redirect( szConvertCommand.c_str() );
			redirect.FlushToLog();
		}
	}

	Log( LogLevel::NORMAL, "Importing entity data...\n" );

	for( const auto& map : appInfo.mapNames )
	{
		const std::string szRipentCommand = 
			std::string( "\"" ) + RipentFilename().u8string() + "\" -import -noinfo \""
			+ destPath.u8string() + FILESYSTEM_PATH_SEPARATOR + FS_MAPS_DIR + map + BSP_EXT + "\"";

		Log( LogLevel::VERBOSE, "Command: %s\n", szRipentCommand.c_str() );
		CRedirectOutput redirect( szRipentCommand.c_str() );
		redirect.FlushToLog();
	}

	Log( LogLevel::NORMAL, "Flushing temporary data...\n" );

	{
		pFileSystem->AddSearchPath( ( destPath / FS_MAPS_DIR ).u8string().c_str(), FS_MAPS_PATH );

		for( const auto& map : appInfo.mapNames )
		{
			const std::string szMapName = map + ENT_EXT;

			pFileSystem->RemoveFile( szMapName.c_str(), FS_MAPS_PATH );
		}

		pFileSystem->RemoveAllSearchPaths();
	}

	Log( LogLevel::NORMAL, "Finished!\n" );

	return uiFailedCount == 0;
}
