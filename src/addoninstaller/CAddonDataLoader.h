#ifndef ADDONINSTALLER_CADDONDATALOADER_H
#define ADDONINSTALLER_CADDONDATALOADER_H

#include <vector>
#include <string>
#include <fstream>

class CAppInfo;

/**
*	Loads the addons text file and parses it into usable data.
*/
class CAddonDataLoader final
{
public:
	CAddonDataLoader();
	~CAddonDataLoader();

	const std::vector<CAppInfo>& GetAppInfos() const { return m_AppInfos; }
	std::vector<CAppInfo>& GetAppInfos() { return m_AppInfos; }

	bool Load( const std::string& szFilename );

private:
	bool LoadAppInfo();

	bool SkipEmptyLines();

	bool ReadLine( std::string& szLine );

	bool TokenMatches( std::string& szLine, const char* const pszToken );

private:
	std::vector<CAppInfo> m_AppInfos;

	std::ifstream m_In;
	size_t m_uiLine;

private:
	CAddonDataLoader( const CAddonDataLoader& ) = delete;
	CAddonDataLoader& operator=( const CAddonDataLoader& ) = delete;
};

#endif //ADDONINSTALLER_CADDONDATALOADER_H
