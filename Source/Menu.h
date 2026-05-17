#pragma once
#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning(disable:ALL_CODE_ANALYSIS_WARNINGS)
#include "raylib.h"
#pragma warning(pop)

#include "RayUtils.h"
#include <memory>
#include <vector>
#include <string>

struct Menu_Icon
{
    virtual ~Menu_Icon() = default;
    virtual void Draw(Rectangle rect, bool selected, bool hovered) const = 0;
};

class Menu
{
public:
    void Add_item(std::unique_ptr<Menu_Icon> item)
    {
        items.push_back(std::move(item));
    }
    
    void Update(Vector2 position) noexcept;
    void Draw(Vector2 position) const;
    int  Selected_index() const noexcept { return selected; }
    void Deselect() noexcept
    {
        selected = -1;
    }
    bool Clicked(Vector2 mouse_position) const noexcept;

private:
    std::vector<std::unique_ptr<Menu_Icon>> items;
    Rectangle Icon(Vector2 position, size_t index) const noexcept;
    int selected = -1;
};

struct Rect_list
{
    std::vector<std::string> list;
    Rectangle rectangle;

public:
    void Draw(const Color& color)
    {

        int i = 0;
        for (auto s : list)
        {
            Draw_line(color, i);
            i++;
        }
    }

    void Draw_outline()
    {

        int i = 0;
        for (auto s : list)
        {
            const Rectangle rec = Entry_rectangle(i);
            DrawRectangleLinesEx(rec, 1.0f, GRAY);
            i++;
        }
    }

    void Draw_line(const Color& color, const size_t index)
    {
        const float line_y = rectangle.y + line_height() * index;

        DrawTextEx(Font(), list.at(index).data(), {rectangle.x , line_y}, line_height(), 2, color);

    }

    Rectangle Entry_rectangle(const size_t index) 
    {
        const float height = line_height();
        const float line_y = rectangle.y + height * index;
       

        return TextRect(GetFontDefault(), list.at(index).data(), {rectangle.x , line_y}, height, 2);
    }

private:

    const float line_height() noexcept
    {
        return rectangle.height / list.size();
    }
};