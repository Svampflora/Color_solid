#pragma once

#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning(disable:ALL_CODE_ANALYSIS_WARNINGS)
#include "raylib.h"
#pragma warning(pop)
#include <vector>
#include "Menu.h" 

inline size_t MAX_COATS = 10;


struct Paint
{
    std::string name;
    Color color;
    size_t coats;
    float m2_per_liter;

    Paint(Color _color) noexcept;

    void Draw_swatch(Rectangle rect) const noexcept;
    void Draw_swatch_with_coats(Rectangle rect) const noexcept;
    void Draw_info(Rectangle rect, float liters_of, const Font& font) const noexcept;

};


struct Paintable
{

    virtual ~Paintable() = default;
    virtual float Liters_used(const Paint* paint) const = 0;

};

struct Paint_Icon : Menu_Icon
{
    const Paint* paint;
    const Paintable& object;

    explicit Paint_Icon(const Paint* p, const Paintable& o) noexcept  : 
    paint(p),
    object(o)
    {}

    void Draw(Rectangle rect, bool selected, bool hovered, Font& font) const override;

};
