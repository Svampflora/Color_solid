#pragma once

#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning(disable:ALL_CODE_ANALYSIS_WARNINGS)
#include "raylib.h"
#pragma warning(pop)

#include "State.h"
#include "Room.h"
#include "CameraController.h"
#include "ColorUtils.h"

struct Camera3D;



class Editor : public State
{
	CameraController& camera_controller;
    Room& room;
    Color_solid solid;
    Handle handle;
    std::vector<Paint> paints;
    Menu paint_menu;
    Font font; 

    float min_size = 1.0f;
    float max_size = 10.0f;

public:

    Editor(Room& room_ref, CameraController& camera_controller_ref);
    std::unique_ptr<State> Update() override;
    void Render() const override;

private:
    Handle Make_handle(const Wall* wall);
    const Wall* Hovered_wall() const;
    Wall* Hovered_handle();
    Wall* Hovered_wall();
    const Paint* Selected_paint() const;
    Paint* Selected_paint();

    
    void Edit();
    void Select_handle();
    void Build_paint_menu();
    void Select_paint() noexcept;
    void Paint_surface();
    void Drag_handles();
    void Draw_UI() const;
};