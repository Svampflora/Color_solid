#pragma once
#include "raylib.h"
#include <string>


class Window
{

public:
    Window(std::string title, int screen_width, int screen_height) noexcept
    {
        InitWindow(screen_width, screen_height, title.data());
        SetTargetFPS(60);
    }
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(Window&&) = delete;

    ~Window()
    {
        CloseWindow();
    }
};