#include <type_traits>

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>

#include <GL/glew.h>

#include "Common.h"
#include "Logging.h"

#include "CFontManager.h"

namespace font
{
bool CFontManager::Initialize()
{
	m_HDC = ::CreateCompatibleDC( NULL );

	if( m_HDC == NULL )
	{
		Msg( "CFontManager::Initialize: Failed to create handle to Device Context\n" );
		return false;
	}

	return true;
}

void CFontManager::Shutdown()
{
	if( m_HDC )
	{
		DeleteDC( m_HDC );
		m_HDC = NULL;
	}
}

CFont* CFontManager::FindFont( const char* pszFaceName, const unsigned int uiHeight, const unsigned int uiWidth )
{
	ASSERT( pszFaceName );

	const float flHeight = static_cast<float>( uiHeight );
	const float flWidth = static_cast<float>( uiWidth );

	for( const auto& font : m_Fonts )
	{
		if( font->Equals( pszFaceName, flHeight, flWidth ) )
			return font.get();
	}

	return nullptr;
}

inline int NextP2( int iValue )
{
	int rVal = 1;

	while( rVal < iValue )
		rVal <<= 1;

	return rVal;
}

bool MakeDList( FT_Face face, char ch, float flHeight, GLuint list_base, GLuint* pTexBase )
{
	const auto charIndex = FT_Get_Char_Index( face, ch );

	if( FT_Load_Glyph( face, charIndex, FT_LOAD_DEFAULT ) )
		return false;

	FT_Glyph glyph;

	if( FT_Get_Glyph( face->glyph, &glyph ) )
		return false;

	FT_Glyph_To_Bitmap( &glyph, ft_render_mode_normal, nullptr, true );
	FT_BitmapGlyph bitmap_glyph = reinterpret_cast<FT_BitmapGlyph>( glyph );

	FT_Bitmap& bitmap = bitmap_glyph->bitmap;

	const unsigned int uiWidth		= NextP2( bitmap.width );
	const unsigned int uiHeight		= NextP2( bitmap.rows );

	auto expanded_data = std::make_unique<GLubyte[]>( 2 * uiWidth * uiHeight );

	for( unsigned int uiRow = 0; uiRow < uiHeight; ++uiRow )
	{
		for( unsigned int uiCol = 0; uiCol < uiWidth; ++uiCol )
		{
			expanded_data[ 2 * ( uiCol + uiRow * uiWidth ) ] = 255;
			expanded_data[ 2 * ( uiCol + uiRow * uiWidth ) + 1 ] = 
				uiCol >= bitmap.width || uiRow >= bitmap.rows ? 0 :
				bitmap.buffer[ uiCol + bitmap.width * uiRow ];
		}
	}

	const size_t uiIndex = ch;

	glBindTexture( GL_TEXTURE_2D, pTexBase[ uiIndex ] );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, uiWidth, uiHeight, 0,
				  GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, expanded_data.get() );

	glNewList( list_base + uiIndex, GL_COMPILE );

	glBindTexture( GL_TEXTURE_2D, pTexBase[ uiIndex ] );

	glPushMatrix();

	glTranslatef( static_cast<GLfloat>( bitmap_glyph->left ), 0, 0 );
	////Note: the cast to float is needed because signed int - unsigned int will cause the result to be incorrect.
	////This causes the glyph to render off-screen. - Solokiller
	glTranslatef( 0, face->size->metrics.height / 100.0 + -static_cast<GLfloat>( bitmap_glyph->top ), 0 );

	float x = static_cast<float>( bitmap.width ) / uiWidth;
	float y = static_cast<float>( bitmap.rows ) / uiHeight;

	glBegin( GL_QUADS );
	glTexCoord2d( 0, y );
	glVertex2f( 0, static_cast<GLfloat>( bitmap.rows ) );

	glTexCoord2d( 0, 0 );
	glVertex2f( 0, 0 );

	glTexCoord2d( x, 0 );
	glVertex2f( static_cast<GLfloat>( bitmap.width ), 0 );

	glTexCoord2d( x, y );
	glVertex2f( static_cast<GLfloat>( bitmap.width ), static_cast<GLfloat>( bitmap.rows ) );

	glEnd();

	glPopMatrix();

	glTranslatef( static_cast<GLfloat>( face->glyph->advance.x >> 6 ), 0, 0 );

	glEndList();

	return true;
}

CFont* CFontManager::LoadFont( const char* pszFaceName, const unsigned int uiHeight, const unsigned int uiWidth )
{
	ASSERT( pszFaceName );

	if( auto pFont = FindFont( pszFaceName, uiHeight, uiWidth ) )
		return pFont;

	HFONT hFont = CreateFontA( uiHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, 0, 0, 3, DEFAULT_PITCH | FF_DONTCARE, pszFaceName );

	if( hFont == NULL )
		return nullptr;

	auto hOldObject = SelectObject( m_HDC, hFont );

	const auto size = GetFontData( m_HDC, 0, 0, nullptr, 0 );

	auto buffer = std::make_unique<uint8_t[]>( size );

	const auto read = GetFontData( m_HDC, 0, 0, buffer.get(), size );

	SelectObject( m_HDC, hOldObject );

	DeleteObject( hFont );

	std::unique_ptr<std::remove_pointer<FT_Library>::type, FT_Error ( * )( FT_Library )> library( nullptr, FT_Done_FreeType );

	{
		FT_Library lib;

		if( FT_Init_FreeType( &lib ) )
			return nullptr;

		library.reset( lib );
	}

	std::unique_ptr<std::remove_pointer<FT_Face>::type, FT_Error ( * )( FT_Face )> face( nullptr, FT_Done_Face );

	{
		FT_Face f;

		if( FT_New_Memory_Face( library.get(), buffer.get(), size, 0, &f ) )
			return nullptr;

		face.reset( f );
	}

	FT_Set_Char_Size( face.get(), uiHeight << 6, uiHeight << 6, 96, 96 );

	const size_t uiNumChars = 128;

	GLuint list_base = glGenLists( uiNumChars );

	auto textures = std::make_unique<GLuint[]>( uiNumChars );

	glGenTextures( uiNumChars, textures.get() );

	const bool bSuccess = [ & ]()
	{
		for( unsigned char ch = 0; ch < uiNumChars - 1; ++ch )
		{
			if( !MakeDList( face.get(), ch, static_cast<float>( uiHeight ), list_base, textures.get() ) )
				return false;
		}

		return true;
	}();

	if( !bSuccess )
	{
		glDeleteLists( list_base, uiNumChars );
		glDeleteTextures( uiNumChars, textures.get() );

		return nullptr;
	}

	m_Fonts.emplace_back( 
		std::make_unique<CFont>( 
			pszFaceName, 
			static_cast<float>( uiHeight ), static_cast<float>( uiWidth ), 
			uiNumChars, list_base, std::move( textures ) ) );

	return m_Fonts.back().get();
}
}
