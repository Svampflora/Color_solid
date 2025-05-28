#pragma once
#include "Settings.h"
#include "Window.h"
#include "Startscreen.h"
#include "Gameplay.h"
#include "Endscreen.h"

class Game
{
	Window window{ "Color Solid", SCREEN_WIDTH, SCREEN_HEIGHT };
	std::unique_ptr<State> current_state = std::make_unique<Start_screen>();
	Score_data scoreData{};

public:
	void Update();
	void Render() const noexcept;
};