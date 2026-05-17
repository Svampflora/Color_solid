#pragma once
#include "State.h"
#include "Color_picker.h"
#include "Menu.h"
#include "Project.h"
#include "CameraController.h"


class PaintEditor : public State
{
	Color_picker color_picker;
	Menu paint_menu;
	Project& project;
	CameraController& camera_controller;

public:
	PaintEditor(Project& project_ref, CameraController& camera_ref);
	std::unique_ptr<State> Update() override;
	void Render() const override;

private:
	void Build_paint_menu();
	std::vector<std::string> Paint_stats(const Paint& paint);
};
