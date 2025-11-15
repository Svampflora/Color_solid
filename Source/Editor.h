#pragma once

#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning(disable:ALL_CODE_ANALYSIS_WARNINGS)
#include "raylib.h"
#pragma warning(pop)

#include "State.h"
#include "Room.h"
#include "CameraController.h"

struct Camera3D;

class Editor : public State
{
	CameraController& camera_controller;
    Room& room;
    Wall::Handle handle{};
    std::vector<Paint> paints;
    Paint* selected_paint = nullptr;

    float min_size = 1.0f;
    float max_size = 10.0f;

public:

    Editor(Room& room_ref, CameraController& camera_controller_ref);
    std::unique_ptr<State> Update() override;
    void Render() const override;

private:
    Wall* Hovered_handle();
    Wall* Hovered_wall();
    const Wall* Hovered_wall() const;
    void Edit();
    void Paint_selection() noexcept;
    void Draw_UI() const;
};