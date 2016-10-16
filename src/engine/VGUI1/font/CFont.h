#ifndef ENGINE_VGUI1_FONT_CFONT_H
#define ENGINE_VGUI1_FONT_CFONT_H

#include <memory>

#include <GL/glew.h>

namespace vgui
{
/**
*	VGUI1 font.
*/
class CFont final
{
public:
	CFont( const int iFontID, const float flHeight, const size_t uiNumChars, GLuint listBase, std::unique_ptr<GLuint[]>&& textures );
	~CFont();

	CFont( CFont&& other ) = default;
	CFont& operator=( CFont&& other ) = default;

	int GetID() const { return m_iFontID; }

	float GetHeight() const { return m_flHeight; }

	size_t GetNumChars() const { return m_uiNumChars; }

	GLuint GetListBase() const { return m_ListBase; }

	const GLuint* GetTextures() const { return m_Textures.get(); }

private:
	const int m_iFontID;
	const float m_flHeight;
	const size_t m_uiNumChars;

	GLuint m_ListBase;

	std::unique_ptr<GLuint[]> m_Textures;

private:
	CFont( const CFont& ) = delete;
	CFont& operator=( const CFont& ) = delete;
};
}

#endif //ENGINE_VGUI1_FONT_CFONT_H
