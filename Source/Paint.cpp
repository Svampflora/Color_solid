#include "Paint.h"

#include "Utilities.h"
#include "ColorUtils.h"



Paint::Paint(Color _color) noexcept :
    color(_color),
    coats(2),
    m2_per_liter(10.0f)
{}

void Paint::Draw_info(Rectangle rect, float liters_of) const noexcept
{
    DrawTextF(TextFormat("%.1f L", liters_of), rect.x + (1.5f * rect.width), rect.y, 25, RAYWHITE);
    DrawTextF(TextFormat("%.1f M2 per liter", m2_per_liter), rect.x + (1.5f * rect.width), rect.y + 30, 25, RAYWHITE);
}

void Paint::Draw_swatch(Rectangle rect) const noexcept
{
    DrawRectangleRounded(rect, 0.5f, 10, color);

}

void Paint::Draw_swatch_with_coats(Rectangle rect) const noexcept
{
    for (int coat = narrow_cast<int>(coats) - 1; coat >= 0; --coat)
    {
        Rectangle layer = rect;

        layer.x += coat * layer.width * 0.25f;

        const float alpha =
            1.0f - ((coat + 1.0f) / MAX_COATS);

        const Color fade = HSV_lerp( BLACK, color, alpha);
        DrawRectangleRounded(layer, 0.5f, 10, fade);
    }
    
}
