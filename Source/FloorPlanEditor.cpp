#include "FloorPlanEditor.h"

#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning(disable:ALL_CODE_ANALYSIS_WARNINGS)
#include "raylib.h"
#pragma warning(pop)

#include "Startscreen.h"
#include "Editor.h"
#include <fstream>
#include <iostream>



FloorPlanEditor::FloorPlanEditor(Project& project_ref, CameraController& camera_ref) :
	project(project_ref), 
	camera_controller(camera_ref) 
{
	camera_controller.Set_top_down();
	camera_controller.Set_projection(CAMERA_ORTHOGRAPHIC);
}

std::unique_ptr<State> FloorPlanEditor::Update()
{
	if (IsKeyReleased(KEY_TAB))
	{
		return std::make_unique<Editor>(project, camera_controller);
	}

	//camera_controller.Mouse_scroll_zoom();
	//Camera2D c;

	return nullptr;
}

void FloorPlanEditor::Render() const
{

	camera_controller.Begin_3D();

	project.room.walls.at(project.room.floor_index).Draw_outline(WHITE); //TODO: Demeter problem

	camera_controller.End_3D();

}

