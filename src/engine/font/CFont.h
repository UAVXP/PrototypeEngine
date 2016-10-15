#ifndef ENGINE_FONT_CFONT_H
#define ENGINE_FONT_CFONT_H

#include <memory>

#include <GL/glew.h>

#include "Platform.h"

namespace font
{
class CFont final
{
public:
	CFont( const char* pszName, const float flHeight, const size_t uiCharCount, GLuint listBase, std::unique_ptr<GLuint[]>&& textures );
	~CFont();

	const char* GetName() const { return m_szName; }

	float GetHeight() const { return m_flHeight; }

	size_t GetCharacterCount() const { return m_uiCharCount; }

	GLuint GetListBase() const { return m_ListBase; }

	const GLuint* GetTextures() const { return m_Textures.get(); }

	bool Equals( const char* pszName, const float flHeight ) const;

private:
	char m_szName[ MAX_PATH ];

	const float m_flHeight;

	const size_t m_uiCharCount;

	GLuint m_ListBase;
	std::unique_ptr<GLuint[]> m_Textures;

private:
	CFont( const CFont& ) = delete;
	CFont& operator=( const CFont& ) = delete;
};
}

#endif //ENGINE_FONT_CFONT_H
