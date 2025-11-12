#pragma once
#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning(disable:ALL_CODE_ANALYSIS_WARNINGS)
#include "raylib.h"
#pragma warning(pop)
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