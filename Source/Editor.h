#pragma once

#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning(disable:ALL_CODE_ANALYSIS_WARNINGS)
#include "raylib.h"
#pragma warning(pop)

#include "State.h"
#include "Project.h"
#include "CameraController.h"


struct Camera3D;

struct Feature_settings
{
    Aperture aperture{};
    Aperture entrance{};

};

class Tool
{
public:
    virtual ~Tool() = default;

    virtual const char* Name() const = 0;

    virtual void OnActivate() {}
    virtual void OnDeactivate() {}

    virtual void Update(const Camera& camera, Project& project) = 0;
    virtual void DrawOverlay() const {}
};

class Add_Door : public Tool
{
public:
    const char* Name() const override { return "Door"; }

    void Update(const Camera& camera, Project& project) override
    {
        Ray ray = GetMouseRay(GetMousePosition(), camera);
        Wall* wall = project.room.Hovered_wall(camera, ray);
        if (!wall) return;

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            RayCollision collision = RayIntersectsWall(ray, *wall); 
            float localX = wall->Normalized_coordinate(collision.point).x;
            wall->doors.emplace_back(localX, wall->Height());
        }
    }
};

//struct Tool_Icon : Menu_Icon
//{
//    const Tool* tool;
//
//    explicit Tool_Icon(const Tool* t) noexcept :
//        tool(t)
//    {}
//
//    void Draw(Rectangle rect, bool selected) const override
//    {
//        tool->(rect);
//
//        if (selected)
//            DrawRectangleRoundedLines(rect, 0.5f, 10, 20.0f, DARKGRAY);
//
//        tool->Draw_info(rect);
//    }
//};

class Editor : public State
{
    Project& project;
	CameraController& camera_controller;
    Handle handle;
    Menu paint_menu;
    Feature_settings feature_settings;
    Menu aperture_menu;
    Font font; 

    float min_size = 1.0f; //TODO: move. settings?
    float max_size = 10.0f;

public:

    Editor(Project& project_ref, CameraController& camera_controller_ref);
    std::unique_ptr<State> Update() override;
    void Render() const override;

private:
    Handle Make_handle(const Wall* wall);
    //const Wall* Hovered_wall() const;
    Wall* Hovered_handle();
    Wall* Hovered_wall();
    const Paint* Selected_paint() const;
    Paint* Selected_paint();

    
    void Edit();
    void Build_paint_menu();
    //void Build_aperture_menu();
    void Select_handle();
    void Select_paint() noexcept;
    void Paint_surface();
    void Drag_handles();
    void Draw_UI() const;
};