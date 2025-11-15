#pragma once

#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning(disable:ALL_CODE_ANALYSIS_WARNINGS)
#include "raylib.h"
#pragma warning(pop)

struct Paint
{
    Color color;
    size_t coats;
    float m2_per_liter;

    Paint();

};