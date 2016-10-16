#ifndef ENGINE_FONT_FONTRENDERING_H
#define ENGINE_FONT_FONTRENDERING_H

namespace vgui
{
class CFont;
}

namespace font
{
class CFont;

namespace rendering
{
void Print( const CFont& font, float flX, float flY, const char* pszString );

void PrintVGUI1( const vgui::CFont& font, float flX, float flY, const char* pszString );
}
}

#endif //ENGINE_FONT_FONTRENDERING_H
