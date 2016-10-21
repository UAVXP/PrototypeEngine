#include <algorithm>
#include <cctype>
#include <cstdint>

#include <VGUI_App.h>
#include <VGUI_Panel.h>
#include <VGUI_Font.h>
#include <VGUI_BaseFontPlat.h>
#include <VGUI_BitmapTGA.h>
#include <VGUI_FileInputStream.h>
#include <VGUI_ImagePanel.h>
#include "VGUI1/VGUI_FontPlat.h"

#include "Engine.h"
#include "Logging.h"

#include "FileSystem2.h"

#include "VGUI1/font/CFont.h"
#include "font/FontRendering.h"

#include "CVGUI1Surface.h"

SDL_Cursor* CVGUI1Surface::m_Cursors[ 20 ] = {};

CVGUI1Surface::CVGUI1Surface( vgui::Panel* embeddedPanel )
	: vgui::SurfaceBase( embeddedPanel )
{
	char szPath[ MAX_PATH ];

	if( g_pFileSystem->GetLocalPath( "gfx/vgui/mouse.tga", szPath, sizeof( szPath ) ) )
	{
		vgui::FileInputStream stream( szPath, false );

		m_pEmulatedMouseImage = new vgui::BitmapTGA( &stream, true );

		stream.close();
	}

	if( !SDL_WasInit( SDL_INIT_VIDEO ) )
	{
		SDL_SetHint( "SDL_VIDEO_X11_XRANDR", "1" );
		SDL_SetHint( "SDL_VIDEO_X11_XVIDMODE", "1" );
		SDL_InitSubSystem( SDL_INIT_VIDEO );
	}

	for( int iCursor = SDL_SYSTEM_CURSOR_ARROW; iCursor < SDL_NUM_SYSTEM_CURSORS; ++iCursor )
	{
		m_Cursors[ iCursor + 2 ] = SDL_CreateSystemCursor( static_cast<SDL_SystemCursor>( SDL_SYSTEM_CURSOR_ARROW + iCursor ) );
	}

	m_pCursor = m_Cursors[ SDL_SYSTEM_CURSOR_ARROW + 2 ];

	m_bCursorVisible = true;
}

CVGUI1Surface::~CVGUI1Surface()
{
	m_pCursor = nullptr;

	for( auto& pCursor : m_Cursors )
	{
		if( pCursor )
		{
			SDL_FreeCursor( pCursor );
			pCursor = nullptr;
		}
	}

	if( m_pEmulatedMouseImage )
	{
		delete m_pEmulatedMouseImage;
		m_pEmulatedMouseImage = nullptr;
	}
}

void CVGUI1Surface::setTitle( const char* title )
{
	//Nothing
}

bool CVGUI1Surface::setFullscreenMode( int wide, int tall, int bpp )
{
	//Nothing

	return false;
}

void CVGUI1Surface::setWindowedMode()
{
	//Nothing
}

void CVGUI1Surface::setAsTopMost( bool state )
{
	//Nothing
}

void CVGUI1Surface::createPopup( vgui::Panel* embeddedPanel )
{
	//Nothing
}

bool CVGUI1Surface::hasFocus()
{
	//Nothing

	return true;
}

bool CVGUI1Surface::isWithin( int x, int y )
{
	//Nothing

	return true;
}

int CVGUI1Surface::createNewTextureID()
{
	GLuint texture;

	glGenTextures( 1, &texture );

	return texture;
}

void CVGUI1Surface::GetMousePos( int &x, int &y )
{
	SDL_GetMouseState( &x, &y );

	//TODO: fullscreen mode has to adjust the coordinates to the current game resolution. - Solokiller
}

void CVGUI1Surface::drawSetColor( int r, int g, int b, int a )
{
	m_DrawColor[ 0 ] = r;
	m_DrawColor[ 1 ] = g;
	m_DrawColor[ 2 ] = b;
	m_DrawColor[ 3 ] = a;
}

void CVGUI1Surface::drawFilledRect( int x0, int y0, int x1, int y1 )
{
	glDisable( GL_TEXTURE_2D );

	glColor4ubv( m_DrawColor );

	glBegin( GL_TRIANGLE_STRIP );

		glVertex2f( static_cast<GLfloat>( x0 ), static_cast<GLfloat>( y0 ) );
		glVertex2f( static_cast<GLfloat>( x1 ), static_cast<GLfloat>( y0 ) );
		glVertex2f( static_cast<GLfloat>( x0 ), static_cast<GLfloat>( y1 ) );
		glVertex2f( static_cast<GLfloat>( x1 ), static_cast<GLfloat>( y1 ) );

	glEnd();

	glEnable( GL_TEXTURE_2D );
}

void CVGUI1Surface::drawOutlinedRect( int x0, int y0, int x1, int y1 )
{
	glDisable( GL_TEXTURE_2D );

	glColor4ubv( m_DrawColor );

	glLineWidth( 1 );

	glBegin( GL_LINE_STRIP );

		glVertex2f( static_cast<GLfloat>( x0 ), static_cast<GLfloat>( y0 ) );
		glVertex2f( static_cast<GLfloat>( x1 ), static_cast<GLfloat>( y0 ) );
		glVertex2f( static_cast<GLfloat>( x1 ), static_cast<GLfloat>( y1 ) );
		glVertex2f( static_cast<GLfloat>( x0 ), static_cast<GLfloat>( y1 ) );
		glVertex2f( static_cast<GLfloat>( x0 ), static_cast<GLfloat>( y0 ) );

	glEnd();

	glEnable( GL_TEXTURE_2D );
}

static const size_t FONT_BITMAP_SIZE = 1024;

static uchar g_FontRGBA[ FONT_BITMAP_SIZE * FONT_BITMAP_SIZE ];

inline int NextP2( int iValue )
{
	int rVal = 1;

	while( rVal < iValue )
		rVal <<= 1;

	return rVal;
}

bool MakeDList( char ch, uchar* rgba, const unsigned int uiFontWidth, float flHeight, GLuint list_base, GLuint* pTexBase )
{
	const unsigned int uiDataWidth = uiFontWidth;
	const unsigned int uiDataHeight = static_cast<unsigned int>( flHeight );

	const unsigned int uiWidth = NextP2( uiDataWidth );
	const unsigned int uiHeight = NextP2( uiDataHeight );

	auto expanded_data = std::make_unique<GLubyte[]>( 4 * uiWidth * uiHeight );

	for( unsigned int uiRow = 0; uiRow < uiHeight; ++uiRow )
	{
		for( unsigned int uiCol = 0; uiCol < uiWidth; ++uiCol )
		{
			for( unsigned int uiChannel = 0; uiChannel < 4; ++uiChannel )
			{
				expanded_data[ 4 * ( uiCol + uiRow * uiWidth ) + uiChannel ] =
					uiCol >= uiDataWidth || uiRow >= uiDataHeight ? 0 :
					rgba[ 4 * ( uiCol + ( FONT_BITMAP_SIZE / 2 ) * uiRow ) + uiChannel ];
			}
		}
	}

	const size_t uiIndex = ch;

	glBindTexture( GL_TEXTURE_2D, pTexBase[ uiIndex ] );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, uiWidth, uiHeight, 0,
				  GL_RGBA, GL_UNSIGNED_BYTE, expanded_data.get() );

	glNewList( list_base + uiIndex, GL_COMPILE );

	glBindTexture( GL_TEXTURE_2D, pTexBase[ uiIndex ] );

	glPushMatrix();

	//glTranslatef( static_cast<GLfloat>( bitmap_glyph->left ), 0, 0 );
	////Note: the cast to float is needed because signed int - unsigned int will cause the result to be incorrect.
	////This causes the glyph to render off-screen. - Solokiller
	//glTranslatef( 0, face->size->metrics.height / 100.0 + -static_cast<GLfloat>( bitmap_glyph->top ), 0 );

	float x = static_cast<float>( uiDataWidth ) / uiWidth;
	float y = static_cast<float>( uiDataHeight ) / uiHeight;

	glBegin( GL_QUADS );
	glTexCoord2d( 0, y );
	glVertex2f( 0, static_cast<GLfloat>( uiDataHeight ) );

	glTexCoord2d( 0, 0 );
	glVertex2f( 0, 0 );

	glTexCoord2d( x, 0 );
	glVertex2f( static_cast<GLfloat>( uiDataWidth ), 0 );

	glTexCoord2d( x, y );
	glVertex2f( static_cast<GLfloat>( uiDataWidth ), static_cast<GLfloat>( uiDataHeight ) );

	glEnd();

	glPopMatrix();

	//glTranslatef( static_cast<GLfloat>( face->glyph->advance.x >> 6 ), 0, 0 );

	glEndList();

	return true;
}

void CVGUI1Surface::drawSetTextFont( vgui::Font* font )
{
	if( font )
	{
		auto it = std::find_if( m_Fonts.begin(), m_Fonts.end(), 
			[ = ]( const auto& vguiFont )
			{
				return vguiFont->GetID() == font->getId();
			}
		);

		if( it != m_Fonts.end() )
		{
			m_pActiveFont = it->get();
			return;
		}

		//New font, load it.

		auto plat = font->getPlat();

		const size_t uiNumChars = 256;

		auto listBase = glGenLists( uiNumChars );

		auto textures = std::make_unique<GLuint[]>( uiNumChars );

		glGenTextures( uiNumChars, textures.get() );

#ifndef WIN32
		if( plat->getTall() <= FONT_BITMAP_SIZE / 2 && plat->getTall() > 0 && 
			plat->getWide() <= FONT_BITMAP_SIZE / 2 && plat->getWide() > 0 )
		{
#endif

			int a, b = 0, c;

			for( int ch = 0; ch < uiNumChars; ++ch )
			{
				plat->getCharABCwide( ch, a, b, c );

				//Skip unprintable characters. Note: the textures array zero initializes, so glDeleteTextures calls will be no-ops for these. - Solokiller
				if( !isprint( ch ) )
					continue;

				//memset( g_FontRGBA, 0, sizeof( g_FontRGBA ) );

				plat->getCharRGBA( ch, 0, 0, FONT_BITMAP_SIZE / 2, FONT_BITMAP_SIZE / 2, g_FontRGBA );

				const unsigned int uiWidth = 
	#ifdef WIN32
					b
	#else
					plat->getWide()
	#endif
					;

				if( !MakeDList( ch, g_FontRGBA, uiWidth, static_cast<float>( plat->getTall() ), listBase, textures.get() ) )
				{
					glDeleteLists( listBase, uiNumChars );
					glDeleteTextures( uiNumChars, textures.get() );
					return;
				}
			}

#ifndef WIN32
		}
		else
		{
			//Sanity check. On Linux TrueType fonts don't work and return garbage from these 2 methods.
			//We can't possibly detect this reliably, but this will prevent the game from freezing while loading.
			//It'll load a blank font instead. - Solokiller
			Msg( "CVGUI1Surface::drawSetTextFont: Font has invalid size (x:%d;y:%d)(probably a TrueType font on Linux; this is not supported!)\n",
					plat->getWide(), plat->getTall() );
		}
#endif

		auto vguiFont = std::make_unique<vgui::CFont>( font->getId(), static_cast<float>( font->getTall() ), uiNumChars, listBase, std::move( textures ) );

		m_Fonts.emplace_back( std::move( vguiFont ) );

		m_pActiveFont = m_Fonts.back().get();
	}
	else
	{
		m_pActiveFont = nullptr;
	}
}

void CVGUI1Surface::drawSetTextColor( int r, int g, int b, int a )
{
	m_TextDrawColor[ 0 ] = r;
	m_TextDrawColor[ 1 ] = g;
	m_TextDrawColor[ 2 ] = b;
	m_TextDrawColor[ 3 ] = a;
}

void CVGUI1Surface::drawSetTextPos( int x, int y )
{
	m_iTextXPos = x;
	m_iTextYPos = y;
}

void CVGUI1Surface::drawPrintText( const char* text, int textLen )
{
	//TODO: textLen indicates the number of characters to draw, null terminator is not the intended end! - Solokiller
	//TODO: consider using a default font. - Solokiller
	if( !m_pActiveFont )
		return;

	glColor4ubv( m_TextDrawColor );

	font::rendering::PrintVGUI1( 
		*m_pActiveFont, 
		static_cast<float>( m_iTextXPos ), static_cast<float>( m_iTextYPos ), text );
}

void CVGUI1Surface::drawSetTextureRGBA( int id, const char* rgba, int wide, int tall )
{
	glBindTexture( GL_TEXTURE_2D, id );

	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, wide, tall, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
}

void CVGUI1Surface::drawSetTexture( int id )
{
	m_CurrentTexture = id;

	glBindTexture( GL_TEXTURE_2D, id );
}

void CVGUI1Surface::drawTexturedRect( int x0, int y0, int x1, int y1 )
{
	if( m_CurrentTexture == 0 )
		return;

	glBegin( GL_TRIANGLE_STRIP );

		glTexCoord2f( 0, 0 );
		glVertex2i( x0, y0 );

		glTexCoord2f( 1, 0 );
		glVertex2i( x1, y0 );

		glTexCoord2f( 0, 1 );
		glVertex2i( x0, y1 );

		glTexCoord2f( 1, 1 );
		glVertex2i( x1, y1 );

	glEnd();
}

void CVGUI1Surface::invalidate( vgui::Panel *panel )
{
	//Nothing
}

void CVGUI1Surface::enableMouseCapture( bool state )
{
	//Nothing
}

void CVGUI1Surface::setCursor( vgui::Cursor* cursor )
{
	if( !cursor || m_bCursorLocked )
		return;

	_currentCursor = cursor;

	const auto cursorID = cursor->getDefaultCursor();

	const auto bWasVisible = m_bCursorVisible;

	m_bCursorVisible = cursorID != vgui::Cursor::dc_none;

	_emulatedCursor->setImage( m_pEmulatedMouseImage );

	if( m_bCursorVisible )
	{
		if( cursorID >= vgui::Cursor::dc_none && cursorID < vgui::Cursor::dc_last )
		{
			m_pCursor = m_Cursors[ cursor->getDefaultCursor() ];
		}
	}

	if( m_bCursorVisible )
		SDL_SetCursor( m_pCursor );

	//TODO: need to revisit this when SDL stuff gets implemented more. - Solokiller

	if( !m_bCursorLocked )
	{
		if( !m_bCursorVisible && bWasVisible )
		{
			if( /*m_rawinput.value && BUsesSDLInput() */ false )
				SDL_SetRelativeMouseMode( SDL_TRUE );
			else
				SDL_ShowCursor( false );
		}

		if( m_bCursorVisible && !bWasVisible )
		{
			if( /*m_rawinput.value && BUsesSDLInput() */ false )
				SDL_SetRelativeMouseMode( SDL_FALSE );
			else
				SDL_ShowCursor( true );
		}

		SDL_PumpEvents();

		//Reset the relative mouse state so raw input works properly. - Solokiller
		int x, y;

		SDL_GetRelativeMouseState( &x, &y );
	}
}

void CVGUI1Surface::swapBuffers()
{
	glFinish();

	SDL_GL_SwapWindow( g_Video.GetWindow() );
}

void CVGUI1Surface::pushMakeCurrent( vgui::Panel* panel, bool useInsets )
{
	glMatrixMode( GL_MODELVIEW );

	glPushMatrix();

	glLoadIdentity();

	int x, y, x2, y2;

	panel->getAbsExtents( x, y, x2, y2 );

	int iXOffset = x;
	int iYOffset = y;

	if( useInsets )
	{
		int iXInset, iYInset, iX2, iY2;

		panel->getInset( iXInset, iYInset, iX2, iY2 );

		iXOffset += iXInset;
		iYOffset += iYInset;
	}

	glTranslatef( 
		static_cast<float>( iXOffset ), 
		static_cast<float>( iYOffset ), 
		0 );

	m_iOffsets[ 0 ] = iXOffset;
	m_iOffsets[ 1 ] = iYOffset;

	glEnable( GL_SCISSOR_TEST );
	//TODO: push scissor area on a stack to restore in pop? - Solokiller
	glScissor( x, ( g_Video.GetHeight() - y ) - panel->getTall(), panel->getWide(), panel->getTall() );
}

void CVGUI1Surface::popMakeCurrent( vgui::Panel* panel )
{
	glDisable( GL_SCISSOR_TEST );

	glMatrixMode( GL_MODELVIEW );

	glPopMatrix();
}

void CVGUI1Surface::applyChanges()
{
	//Nothing
}

const vgui::MouseCode SDLMouseCodeToVGUI1[] =
{
	vgui::MOUSE_LAST,		//Not used
	vgui::MOUSE_LEFT,
	vgui::MOUSE_MIDDLE,
	vgui::MOUSE_RIGHT,
	vgui::MOUSE_LAST,		//X1, not used
	vgui::MOUSE_LAST,		//X2, not used
};

void CVGUI1Surface::HandleSDLEvent( SDL_Event& event )
{
	switch( event.type )
	{
	case SDL_MOUSEMOTION:
		{
			vgui::App::getInstance()->internalCursorMoved( event.motion.x, event.motion.y, g_pVGUI1Surface );

			break;
		}

	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
		{
			const auto mouseCode = SDLMouseCodeToVGUI1[ static_cast<size_t>( event.button.button ) ];

			//Unsupported button
			if( mouseCode == vgui::MOUSE_LAST )
				break;

			//TODO: can handle double click here. - Solokiller
			if( event.type == SDL_MOUSEBUTTONDOWN )
				vgui::App::getInstance()->internalMousePressed( mouseCode, g_pVGUI1Surface );
			else
				vgui::App::getInstance()->internalMouseReleased( mouseCode, g_pVGUI1Surface );
			break;
		}

	case SDL_MOUSEWHEEL:
		{
			//TODO: verify that this is correct. This delta may not be what VGUI1 expects. - Solokiller
			vgui::App::getInstance()->internalMouseWheeled( event.wheel.x, g_pVGUI1Surface );

			break;
		}

		//TODO: key events. - Solokiller
	}
}
