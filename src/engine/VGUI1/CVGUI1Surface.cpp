#include <SDL2/SDL.h>

#include <VGUI_Panel.h>
#include <VGUI_Font.h>
#include <VGUI_BaseFontPlat.h>
#include "VGUI1/VGUI_FontPlat.h"

#include "Engine.h"

#include "font/CFont.h"
#include "font/CFontManager.h"
#include "font/FontRendering.h"

#include "CVGUI1Surface.h"

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

void CVGUI1Surface::drawSetTextFont( vgui::Font* font )
{
	if( font )
	{
		auto plat = static_cast<vgui::FontPlat*>( font->getPlat() );

		auto pFont = g_FontManager.LoadFont( plat->m_szName, static_cast<unsigned int>( plat->m_iTall ), static_cast<unsigned int>( plat->m_iWidth ) );

		if( pFont )
			m_pActiveFont = pFont;
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

	font::rendering::Print( *m_pActiveFont, static_cast<float>( m_iTextXPos ), static_cast<float>( m_iTextYPos ), text );
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
	//TODO
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

	int x, y;

	panel->getPos( x, y );

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

	glEnable( GL_SCISSOR_TEST );
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
