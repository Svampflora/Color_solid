#pragma once

#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning(disable:ALL_CODE_ANALYSIS_WARNINGS)
#include "raylib.h"
#pragma warning(pop)
#include <vector>
#include "Menu.h" 

struct Paint
{
    Color color;
    size_t coats;
    float m2_per_liter;

    Paint(Color _color) noexcept;

    void Draw_swatch(Rectangle rect) const noexcept;
    void Draw_info(Rectangle rect, float liters_of) const noexcept;

};

struct PaintMenuItem : Menu_Icon
{
    const Paint* paint;

    explicit PaintMenuItem(const Paint* p) noexcept  : paint(p)  {}

    void Draw(Rectangle rect, bool selected) const noexcept override
    {
        paint->Draw_swatch(rect);

        if (selected)
            DrawRectangleRoundedLines(rect, 0.5f, 10, 20.0f, DARKGRAY);

        
    }
};