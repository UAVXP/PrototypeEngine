#ifndef ENGINE_ENGINE_H
#define ENGINE_ENGINE_H

#include "steam_api.h"

#include "CEngine.h"
#include "CVideo.h"

#include "console/CCommandBuffer.h"
#include "console/CCVarSystem.h"

#include "VGUI1/CVGUI1Surface.h"

#include "font/CFontManager.h"

class IFileSystem2;

/**
*	The command buffer.
*/
extern CCommandBuffer g_CommandBuffer;
extern cvar::CCVarSystem g_CVar;

extern CEngine g_Engine;

extern CVideo g_Video;

extern CVGUI1Surface* g_pVGUI1Surface;

extern IFileSystem2* g_pFileSystem;

extern CSteamAPIContext g_SteamAPIContext;

extern font::CFontManager g_FontManager;

#endif //ENGINE_ENGINE_H
