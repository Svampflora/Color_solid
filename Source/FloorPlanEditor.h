#pragma once
#include "State.h"
#include "Room.h"
#include "CameraController.h"
#include <vector>

class FloorPlanEditor : public State
{
	Room& room;
	CameraController& camera_controller;
public:

	FloorPlanEditor(Room& roomRef, CameraController& camRef);
	std::unique_ptr<State> Update() override;
	void Render() const override;

private:
};