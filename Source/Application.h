#pragma once
#include "Settings.h"
#include "Window.h"
#include "Editor.h"
#include "Color_Picker.h"
#include "FloorPlanEditor.h"
#include "CameraController.h"

class Application
{
	Window window{ "Color Solid", SCREEN_WIDTH, SCREEN_HEIGHT };
	Room room{};
	CameraController camera_controller {room.position};
	std::unique_ptr<State> current_state = std::make_unique<Editor>(room, camera_controller);

public:
	void Update();
	void Render() const;
};