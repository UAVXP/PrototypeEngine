#ifndef ADDONINSTALLER_HELPERS_H
#define ADDONINSTALLER_HELPERS_H

#include <experimental/filesystem>
#include <vector>
#include <string>

#include "steam_api.h"

#include "Platform.h"

namespace fs = std::experimental::filesystem;

const size_t MAX_QUESTION_ATTEMPTS = 10;	//Maximum number of times to ask a question. Avoids infinite loops and such.

const AppId_t SVENCOOP_APPID = 225840;

class CAppInfo final
{
public:
	AppId_t appId;
	std::string szName;
	std::string szModDir;
	std::string szSvenSupportFile;

	bool bIsBShiftMapFormat;

	std::vector<std::string> mapNames;

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
	SILENT	= -1,
	FIRST	= 0,
	ALWAYS	= FIRST,
	NORMAL,
	EXTRA,
	VERBOSE,
	LAST	= VERBOSE
};

extern LogLevel g_LogLevel;

void Log( const LogLevel log, const char* const pszFormat, ... );

char* FixSlashes( char* pszPath );

enum class QuestionAction
{
	ASKAGAIN = 0,
	YES = 1,
	NO = 2
};

QuestionAction AskYNQuestion( const char* const pszQuestion );

bool ValidatePath( const char* const pszPath, const bool bIsDirectory, const bool bSilent = false );
bool DoesFileExist( const char* const pszFilename );
bool AskForDirectory( const char* const pszName, char* pszPath, const size_t uiBufferSize );
bool AskForHLDirectory( std::vector<CAppInfo>& appInfos );

bool AskForDirectories( std::vector<CAppInfo>& appInfos );
bool GetDirectoriesFromSteam( std::vector<CAppInfo>& appInfos );

fs::path RipentFilename();
fs::path UnzipFilename();

bool HasRequiredFiles();

bool CopyGameFiles( const CAppInfo& appInfo );

#endif //ADDONINSTALLER_HELPERS_H
