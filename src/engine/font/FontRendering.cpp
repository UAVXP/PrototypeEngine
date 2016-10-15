#include <memory>
#include <vector>
#include <sstream>
#include <string>

#include "CFont.h"

#include "FontRendering.h"

namespace font
{
namespace rendering
{
// A Fairly Straightforward Function That Pushes
// A Projection Matrix That Will Make Object World
// Coordinates Identical To Window Coordinates.
inline void pushScreenCoordinateMatrix() {
	glPushAttrib( GL_TRANSFORM_BIT );
	GLint   viewport[ 4 ];
	glGetIntegerv( GL_VIEWPORT, viewport );
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D( viewport[ 0 ], viewport[ 2 ], viewport[ 3 ], viewport[ 1 ] );
	glPopAttrib();
}

// Pops The Projection Matrix Without Changing The Current
// MatrixMode.
inline void pop_projection_matrix() {
	glPushAttrib( GL_TRANSFORM_BIT );
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glPopAttrib();
}

void Print( const CFont& font, float flX, float flY, const char* pszString )
{
	pushScreenCoordinateMatrix();

	GLuint fontID = font.GetListBase();

	float flHeight = font.GetHeight() / .63f;

	std::vector<std::string> lines;

	{
		std::stringstream stream( pszString );

		std::string szLine;

		while( std::getline( stream, szLine ) )
		{
			lines.emplace_back( std::move( szLine ) );
		}
	}

	glPushAttrib( GL_LIST_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT | GL_TRANSFORM_BIT );

	glMatrixMode( GL_MODELVIEW );
	glDisable( GL_LIGHTING );
	glEnable( GL_TEXTURE_2D );
	glDisable( GL_DEPTH_TEST );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	glListBase( fontID );

	float modelview_matrix[ 16 ];

	glGetFloatv( GL_MODELVIEW_MATRIX, modelview_matrix );

	for( size_t uiIndex = 0; uiIndex < lines.size(); ++uiIndex )
	{
		glPushMatrix();
		glLoadIdentity();
		glTranslatef( flX, flY + flHeight * ( uiIndex ), 0 );
		glMultMatrixf( modelview_matrix );

		glCallLists( lines[ uiIndex ].length(), GL_UNSIGNED_BYTE, lines[ uiIndex ].c_str() );
		glPopMatrix();
	}

	glPopAttrib();

	pop_projection_matrix();
}
}
}
