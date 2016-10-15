#include <cstring>

#include "Common.h"

#include "CFont.h"

namespace font
{
CFont::CFont( const char* pszName, const float flHeight, const size_t uiCharCount, GLuint listBase, std::unique_ptr<GLuint[]>&& textures )
	: m_flHeight( flHeight )
	, m_uiCharCount( uiCharCount )
	, m_ListBase( listBase )
	, m_Textures( std::move( textures ) )
{
	ASSERT( pszName );

	strncpy( m_szName, pszName, sizeof( m_szName ) );
	m_szName[ sizeof( m_szName ) - 1 ] = '\0';
}

CFont::~CFont()
{
	glDeleteLists( m_ListBase, m_uiCharCount );
	glDeleteTextures( m_uiCharCount, m_Textures.get() );
}

bool CFont::Equals( const char* pszName, const float flHeight ) const
{
	ASSERT( pszName );

	return strcmp( pszName, m_szName ) == 0 && m_flHeight == flHeight;
}
}
