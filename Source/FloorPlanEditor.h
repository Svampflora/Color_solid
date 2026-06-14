#pragma once
#include "State.h"
#include "Project.h"
#include "CameraController.h"
#include <vector>

class FloorPlanEditor : public State
{
	Project&			project;
	CameraController&	camera_controller;
	Font&				font;
public:

	FloorPlanEditor(Project& project_ref, CameraController& camera_ref, Font& _font);
	std::unique_ptr<State> Update() override;
	void Render() const override;

private:
};