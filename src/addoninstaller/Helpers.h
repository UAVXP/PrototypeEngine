#ifndef ADDONINSTALLER_HELPERS_H
#define ADDONINSTALLER_HELPERS_H

#include <experimental/filesystem>
#include <string>
#include <vector>

#include "steam_api.h"

#include "Platform.h"

class ISteamApps;
class IFileSystem2;

namespace fs = std::experimental::filesystem;

const AppId_t SVENCOOP_APPID = 225840;

#define FS_MAPS_DIR "maps" FILESYSTEM_PATH_SEPARATOR
#define FS_SKIES_DIR "gfx" FILESYSTEM_PATH_SEPARATOR "env" FILESYSTEM_PATH_SEPARATOR

#define FS_FROM_PATH "FROM"
#define FS_MAPS_PATH "MAPS"

#define BSP_EXT ".bsp"
#define ENT_EXT ".ent"

class CAppInfo final
{
public:
	/**
	*	The game's app ID.
	*/
	AppId_t appId;

	/**
	*	Name of the game.
	*/
	std::string szName;

	/**
	*	The game's mod directory.
	*/
	std::string szModDir;

	/**
	*	Name of the .sven file that contains support files.
	*/
	std::string szSvenSupportFile;

	/**
	*	Whether this game uses the Blue Shift BSP format.
	*/
	bool bIsBShiftMapFormat;

	/**
	*	Maps to copy and convert.
	*/
	std::vector<std::string> mapNames;

	/**
	*	Path to the game's mod directory. Retrieved from Steam.
	*/
	char szPath[ MAX_PATH ];

	CAppInfo()
		: appId( k_uAppIdInvalid )
		, bIsBShiftMapFormat( false )
	{
		memset( szPath, 0, sizeof( szPath ) );
	}

	CAppInfo( const AppId_t appId,
			   std::string&& szName, std::string&& szModDir, std::string&& szSvenSupportFile,
			   const bool bIsBShiftMapFormat,
			   std::vector<std::string>&& mapNames )
		: appId( appId )
		, szName( szName )
		, szModDir( szModDir )
		, szSvenSupportFile( szSvenSupportFile )
		, bIsBShiftMapFormat( bIsBShiftMapFormat )
		, mapNames( std::move( mapNames ) )
	{
		memset( szPath, 0, sizeof( szPath ) );
	}

	CAppInfo( const CAppInfo& ) = default;
	CAppInfo& operator=( const CAppInfo& ) = default;

	void Init( const AppId_t appId,
			   std::string&& szName, std::string&& szModDir, std::string&& szSvenSupportFile,
			   const bool bIsBShiftMapFormat,
			   std::vector<std::string>&& mapNames )
	{
		this->appId					= std::move( appId );
		this->szName				= std::move( szName );
		this->szModDir				= std::move( szModDir );
		this->szSvenSupportFile		= std::move( szSvenSupportFile );
		this->bIsBShiftMapFormat	= std::move( bIsBShiftMapFormat );
		this->mapNames				= std::move( mapNames );

		memset( szPath, 0, sizeof( szPath ) );
	}
};

enum class LogLevel
{
	FIRST	= 0,
	SILENT	= FIRST,
	ALWAYS,
	NORMAL,
	EXTRA,
	VERBOSE,
	LAST	= VERBOSE
};

extern LogLevel g_LogLevel;

void Log( const LogLevel log, const char* const pszFormat, ... );

enum class QuestionAction
{
	ASKAGAIN = 0,
	YES = 1,
	NO = 2
};

/**
*	Asks a simple Yes or No question.
*	@return Whether the user input YES or NO.
*/
QuestionAction AskYNQuestion( const char* const pszQuestion );

using ExistenceCheckerFn = bool( *)( const fs::path& );

struct RequiredFile_t
{
	const fs::path filename;
	const char* const pszHelpInfo;

	ExistenceCheckerFn existenceChecker;
};

bool ValidatePath( const char* const pszPath, const bool bIsDirectory, const bool bSilent = false );
bool DoesFileExist( const char* const pszFilename );
bool AskForDirectory( const char* const pszName, char* pszPath, const size_t uiBufferSize );

/**
*	Asks for the HL install directory, and validates the existence of the mod directories
*/
bool AskForHLDirectory( std::vector<CAppInfo>& appInfos );

/**
*	Prints the paths of all apps, or "Not Found" if the path is not set.
*/
void PrintPaths( const std::vector<CAppInfo>& appInfos );

/**
*	Asks for the directories to all apps that aren't already known.
*	Returns true if all questions were answered, false otherwise.
*/
bool AskForDirectories( std::vector<CAppInfo>& appInfos );

/**
*	Queries Steam for the mod directories of each app.
*/
bool GetDirectoriesFromSteam( ISteamApps& steamApps, std::vector<CAppInfo>& appInfos );

/**
*	Filename of the ripent tool.
*/
fs::path RipentFilename();

/**
*	Filename of the unzip tool.
*/
fs::path UnzipFilename();

/**
*	Checks whether required files exist. This includes ripent and unzip.
*/
bool HasRequiredFiles();

/**
*	Copies all game files for a given game.
*	This abuses Valve's filesystem to copy maps.
*	Better than writing wrappers ourselves.
*/
bool CopyGameFiles( const fs::path& destPath, IFileSystem2& fileSystem, const CAppInfo& appInfo );

#endif //ADDONINSTALLER_HELPERS_H
