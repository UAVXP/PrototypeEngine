#ifndef ENGINE_VGUI1_VGUI_FONTPLAT_H
#define ENGINE_VGUI1_VGUI_FONTPLAT_H

#include <cstdint>

#include <VGUI_BaseFontPlat.h>

#include "Platform.h"

namespace vgui
{
struct VFontData
{
	int m_CharWidths[ 256 ];
	int m_BitmapCharWidth;
	int m_BitmapCharHeight;
	uint8_t* m_pBitmap;
};

struct ABC_RBTREE
{
	uint8_t data[ 24 ];
};

class FontPlat : public BaseFontPlat
{
public:
#ifdef WIN32
	//The windows layout is incomplete, but we only need the font handle so it's good. - Solokiller
	HFONT m_hFont;
	HDC m_HDC;

	char padding[ 4 * 1046 ];

	char* m_szName;

	int m_iWidth;
	int m_iTall;

	float m_flRotation;
	int m_iWeight;

	bool m_bItalic;
	bool m_bUnderlined;
	bool m_bStrikeout;
	bool m_bSymbol;

#else
	int bufSize[ 2 ];
	uchar* buf;
	VFontData m_BitmapFont;
	bool m_bBitmapFont;

	char m_szName[ 32 ];

	int m_iTall;
	int m_iWeight;
	int m_iFlags;
	bool m_bAntiAliased;
	bool m_bRotary;
	bool m_bAdditive;

	int m_iDropShadowOffset;

	bool m_bUnderlined;

	int m_iOutlineSize;
	int m_iHeight;
	int m_iMaxCharWidth;
	int m_iAscent;
	
	ABC_RBTREE m_ExtendedABCWidthsCache;

	int m_iScanLines;
	int m_iBlur;
	float* m_pGaussianDistribution;
#endif
};


const size_t size = sizeof( FontPlat );

const auto offset = offsetof( FontPlat, m_szName ) / 4;
}

#endif //ENGINE_VGUI1_VGUI_FONTPLAT_H
