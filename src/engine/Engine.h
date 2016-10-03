#ifndef ENGINE_ENGINE_H
#define ENGINE_ENGINE_H

#include "CEngine.h"

#include "console/CCommandBuffer.h"
#include "console/CCVarSystem.h"

#include "VGUI1/CVGUI1Surface.h"

/**
*	The command buffer.
*/
extern CCommandBuffer g_CommandBuffer;
extern cvar::CCVarSystem g_CVar;

extern CEngine g_Engine;

extern CVGUI1Surface* g_pVGUI1Surface;

#endif //ENGINE_ENGINE_H
