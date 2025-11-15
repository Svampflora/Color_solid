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



FloorPlanEditor::FloorPlanEditor(Room& roomRef, CameraController& camRef) :
	room(roomRef), 
	camera_controller(camRef) 
{
	camera_controller.Set_top_down();
}

std::unique_ptr<State> FloorPlanEditor::Update()
{
	if (IsKeyReleased(KEY_TAB))
	{
		return std::make_unique<Editor>(room, camera_controller);
	}

	camera_controller.Update();


	return nullptr;
}

void FloorPlanEditor::Render() const
{
	camera_controller.Begin_3D();

	room.walls.at(room.floor_index).Draw();

	camera_controller.End_3D();

}

