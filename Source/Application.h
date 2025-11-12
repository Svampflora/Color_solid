#pragma once
#include "Settings.h"
#include "Window.h"
#include "Startscreen.h"
#include "Color_Picker.h"
#include "Endscreen.h"

class Application
{
	Window window{ "Color Solid", SCREEN_WIDTH, SCREEN_HEIGHT };
	std::unique_ptr<State> current_state = std::make_unique<Start_screen>();

public:
	void Update();
	void Render() const;
};