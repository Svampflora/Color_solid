#include "Menu.h"

    constexpr Vector2 ICON_SIZE = { 60.0f, 60.0f }; //move to settings
    constexpr float ICON_PADDING = 20.0f;

void Menu::Update(Vector2 position) noexcept
{

    const Vector2 mouse = GetMousePosition();


    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        selected = -1;
    }
    

    const Rectangle entire_menu = { position.x, position.y, ICON_SIZE.x, (items.size() * ICON_SIZE.y) + (items.size() * ICON_PADDING) };
    if (!CheckCollisionPointRec(mouse, entire_menu))
    {
        return;
    }

    for (int i = 0; i < items.size(); ++i)
    {
        const float icon_vertical_offset = (i * ICON_SIZE.y) + (i * ICON_PADDING);
        const Rectangle item_rect
        {
            position.x,
            position.y + icon_vertical_offset,
            ICON_SIZE.x,
            ICON_SIZE.y
        };

        const bool hovered = CheckCollisionPointRec(mouse, item_rect);

        if (hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            selected = i;
        }

    }
    
}

void Menu::Draw(Vector2 position) const
{

    for (int i = 0; i < items.size(); ++i)
    {
        const Rectangle item_rect = Icon(position, i);


        items.at(i)->Draw(item_rect, selected == i);
    }
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
