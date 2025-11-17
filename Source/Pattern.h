#pragma once
#include "RayUtils.h"



struct PatternElement 
{
    float width;   // in meters (e.g. wallpaper 53 cm)
    float height;  // in meters
};

struct PatternInstance {
    PatternElement* element;
    Vector2 offset;       // pattern shift (u,v) in meters
    float rotation;       // 0, 90, 180, 270 degrees (optional)
};