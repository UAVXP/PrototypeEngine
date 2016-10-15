#ifndef ENGINE_FONT_CFONTMANAGER_H
#define ENGINE_FONT_CFONTMANAGER_H

#include <memory>
#include <vector>

#include "Platform.h"

#include "CFont.h"

namespace font
{
class CFontManager final
{
private:
	typedef std::vector<std::unique_ptr<CFont>> Fonts_t;

public:
	CFontManager() = default;
	~CFontManager() = default;

	bool Initialize();

	void Shutdown();

	CFont* FindFont( const char* pszFaceName, const unsigned int uiHeight );

	CFont* LoadFont( const char* pszFaceName, const unsigned int uiHeight );

private:
	Fonts_t m_Fonts;

#ifdef WIN32
	HDC m_HDC = NULL;
#endif

private:
	CFontManager( const CFontManager& ) = delete;
	CFontManager& operator=( const CFontManager& ) = delete;
};
}

#endif //ENGINE_FONT_CFONTMANAGER_H
