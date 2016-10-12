#include "Helpers.h"

#include "CAddonDataLoader.h"

#define ADDONDATA_TOKEN_BEGIN "addon_begin"
#define ADDONDATA_TOKEN_END "addon_end"

CAddonDataLoader::CAddonDataLoader()
{
}

CAddonDataLoader::~CAddonDataLoader()
{
}

bool CAddonDataLoader::Load( const std::string& szFilename )
{
	m_AppInfos.clear();

	m_In.open( szFilename );

	if( !m_In )
		return false;

	Log( LogLevel::EXTRA, "CAddonDataLoader: Opened file %s\n", szFilename.c_str() );

	while( m_In )
	{
		if( !SkipEmptyLines() )
			break;

		if( !LoadAppInfo() )
		{
			m_AppInfos.clear();
			m_In.close();
			return false;
		}
	}

	m_In.close();

	return true;
}

bool CAddonDataLoader::LoadAppInfo()
{
	CAppInfo info;

	std::string szLine;

	if( !TokenMatches( szLine, ADDONDATA_TOKEN_BEGIN ) )
		return false;

	if( !ReadLine( szLine ) )
		return false;

	info.appId = strtoul( szLine.c_str(), nullptr, 10 );

	if( !ReadLine( szLine ) )
		return false;

	info.szName = std::move( szLine );

	if( !ReadLine( szLine ) )
		return false;

	info.szModDir = std::move( szLine );

	if( !ReadLine( szLine ) )
		return false;

	info.szSvenSupportFile = std::move( szLine );

	if( !ReadLine( szLine ) )
		return false;

	info.bIsBShiftMapFormat = szLine == "true";

	while( true )
	{
		if( !ReadLine( szLine ) )
			return false;

		if( szLine == ADDONDATA_TOKEN_END )
			break;

		info.mapNames.push_back( szLine );
	}

	//Success
	m_AppInfos.push_back( std::move( info ) );

	return true;
}

/*
* Skips empty lines
* Returns false if EOF was reached.
*/
bool CAddonDataLoader::SkipEmptyLines()
{
	std::streampos curPos;

	std::string szLine;

	do
	{
		curPos = m_In.tellg();

		if( !std::getline( m_In, szLine ) )
			return false;

		//Check if any other newline characters were at the end, in case of Linux not liking Windows encoding.
		while( !szLine.empty() )
		{
			if( szLine.back() == '\n' || szLine.back() == '\r' )
				szLine.resize( szLine.size() - 1 );
			else
				break;
		}

		if( !szLine.empty() )
			break;

		//Only count lines we've discarded
		++m_uiLine;
	}
	while( true );

	m_In.seekg( curPos );

	return true;
}

bool CAddonDataLoader::ReadLine( std::string& szLine )
{
	++m_uiLine;

	if( !std::getline( m_In, szLine ) )
	{
		Log( LogLevel::ALWAYS, "CAddonDataLoader: Error while reading at line %u\n", m_uiLine );
		return false;
	}

	//Check if any other newline characters were at the end, in case of Linux not liking Windows encoding.
	while( !szLine.empty() )
	{
		if( szLine.back() == '\n' || szLine.back() == '\r' )
			szLine.resize( szLine.size() - 1 );
		else
			break;
	}

	return true;
}

bool CAddonDataLoader::TokenMatches( std::string& szLine, const char* const pszToken )
{
	if( !ReadLine( szLine ) )
		return false;

	if( szLine == pszToken )
		return true;

	Log( LogLevel::ALWAYS, "CAddonDataLoader: Line %u: Expected '%s', got '%s'\n", m_uiLine, pszToken, szLine.c_str() );

	return false;
}