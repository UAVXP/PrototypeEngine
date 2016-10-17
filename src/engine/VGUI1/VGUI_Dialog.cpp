#include "Engine.h"

#include "VGUI_Dialog.h"

namespace vgui
{
void Dialog::CenterOnScreen()
{
	CenterInRegion( g_Video.GetWidth(), g_Video.GetHeight() );
}

void Dialog::CenterOnPanel( Panel* pPanel )
{
	if( pPanel == nullptr )
		CenterOnScreen();
	else
	{
		int x, y, x2, y2;

		pPanel->getAbsExtents( x, y, x2, y2 );

		CenterInRegion( x, y, x2 - x, y2 - y );
	}
}

void Dialog::CenterOnParent()
{
	CenterOnPanel( getParent() );
}

void Dialog::CenterOnPoint( int x, int y )
{
	CenterInRegion( x * 2, y * 2 );
}

void Dialog::CenterInRegion( int wide, int tall )
{
	CenterInRegion( 0, 0, wide, tall );
}

void Dialog::CenterInRegion( int x, int y, int wide, int tall )
{
	int myWide, myTall;

	getSize( myWide, myTall );

	int xDest = x + ( wide - myWide ) / 2;
	int yDest = y + ( tall - myTall ) / 2;

	screenToLocal( xDest, yDest );

	setPos( xDest, yDest );
}
}
