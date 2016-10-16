#include "CFont.h"

namespace vgui
{
CFont::CFont( const int iFontID, const float flHeight, const size_t uiNumChars, GLuint listBase, std::unique_ptr<GLuint[]>&& textures )
	: m_iFontID( iFontID )
	, m_flHeight( flHeight )
	, m_uiNumChars( uiNumChars )
	, m_ListBase( listBase )
	, m_Textures( std::move( textures ) )
{
}

CFont::~CFont()
{
	glDeleteLists( m_ListBase, m_uiNumChars );
	glDeleteTextures( m_uiNumChars, m_Textures.get() );
}
}
