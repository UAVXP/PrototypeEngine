#ifndef COMMON_COMMON_H
#define COMMON_COMMON_H

#include <cassert>

#include "tier1/CommandLine.h"

//TODO: for now, map ASSERT to assert. - Solokiller
#ifndef NDEBUG
#define ASSERT assert
#else
#define ASSERT
#endif

#define BSP_FILE_EXT ".bsp"


typedef unsigned char byte;

#define	MAX_QPATH		64			// max length of a quake game pathname

#endif //COMMON_COMMON_H
