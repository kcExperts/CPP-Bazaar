#ifndef PRECOMP_H
#define PRECOMP_H

#define RAYLIB_NAMESPACE_BEGIN namespace rl {
#define RAYLIB_NAMESPACE_END }

#include <winsock2.h>

#undef DrawText
#undef LoadImage

RAYLIB_NAMESPACE_BEGIN
#include "raylib.h"
RAYLIB_NAMESPACE_END

namespace sst
{
    inline const int baseX = 1280;
    inline const int baseY = 720;

    inline int cx(int x)
    {
        return (((float)rl::GetScreenWidth())/baseX) * (float)x;
    }

    inline int cy(int y)
    {
        return (((float)rl::GetScreenHeight())/baseY) * (float)y;
    }

    inline float cxf(float x)
    {
        return (((float)rl::GetScreenWidth())/baseX) * x;
    }

    inline float cyf(float y)
    {
        return (((float)rl::GetScreenHeight())/baseY) * y;
    }
}



#endif