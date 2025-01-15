#ifndef PRECOMP_H
#define PRECOMP_H

#include "raylib.h"
#include "raymath.h"

namespace sst
{
    inline const int baseX = 1280;
    inline const int baseY = 720;

    inline int cx(int x)
    {
        return (((float)GetScreenWidth())/baseX) * (float)x;
    }

    inline int cy(int y)
    {
        return (((float)GetScreenHeight())/baseY) * (float)y;
    }

    inline float cxf(float x)
    {
        return (((float)GetScreenWidth())/baseX) * x;
    }

    inline float cyf(float y)
    {
        return (((float)GetScreenHeight())/baseY) * y;
    }
}



#endif