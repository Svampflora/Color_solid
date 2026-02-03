#include "Menu.h"

    constexpr Vector2 ICON_SIZE = { 60.0f, 60.0f }; //move to settings
    constexpr float ICON_PADDING = 20.0f;

void Menu::Update(Vector2 position) noexcept
{
    const Vector2 mouse = GetMousePosition();

    if (!Clicked(mouse))
    {
        return;
    }

    for (int i = 0; i < items.size(); ++i)
    {        
        const Rectangle item_rect = Icon(position, i);
        const bool hovered = CheckCollisionPointRec(mouse, item_rect);

        if (hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            selected = i;
        }
    }   
}

void Menu::Draw(Vector2 position) const
{
    const Vector2 mouse = GetMousePosition();

    for (int i = 0; i < items.size(); ++i)
    {
        const Rectangle item_rect = Icon(position, i);
        const bool hovered = CheckCollisionPointRec(mouse, item_rect);

        items.at(i)->Draw(item_rect, selected == i, hovered);

    }
}

bool Menu::Clicked(Vector2 mouse_position) const noexcept
{
    const Rectangle entire_menu = { mouse_position.x, mouse_position.y, ICON_SIZE.x, (items.size() * ICON_SIZE.y) + (items.size() * ICON_PADDING) };

    if (CheckCollisionPointRec(mouse_position, entire_menu))
    {
        return true;
    }
    return false;
}

Rectangle Menu::Icon(Vector2 position, size_t index) const noexcept
{
    const float icon_vertical_offset = (index * ICON_SIZE.y) + (index * ICON_PADDING);

    return 
    {
        position.x,
        position.y + icon_vertical_offset,
        ICON_SIZE.x,
        ICON_SIZE.y
    };
}
