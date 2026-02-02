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

    virtual void Draw_swatch(Rectangle rect) const noexcept = 0;

};

class Add_Door : public Tool
{
public:
    const char* Name() const noexcept override { return "Door"; }

    void Update(const Camera& camera, Project& project) override
    {
        const Ray ray = GetMouseRay(GetMousePosition(), camera);
        Wall* wall = project.room.Hovered_wall(camera, ray);
        if (!wall) return;

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            const RayCollision collision = RayIntersectsWall(ray, *wall); 
            float localX = wall->Normalized_coordinate(collision.point).x;
            wall->doors.emplace_back(localX, wall->Height());
        }
    }

    void Draw_swatch(Rectangle rect) const noexcept override;

};



class Editor : public State
{
    Project& project;
	CameraController& camera_controller;
    Handle handle;
    Menu paint_menu;
    Feature_settings feature_settings;
    std::vector<std::unique_ptr<Tool>> tools;
    Tool* active_tool = nullptr;
    Menu tool_menu;
    Font font; 

    float min_size = 1.0f; //TODO: move. settings?
    float max_size = 10.0f;

public:

    Editor(Project& project_ref, CameraController& camera_controller_ref);
    std::unique_ptr<State> Update() override;
    void Render() const override;

    Tool& Get_tool(size_t i)
    {
        return *tools.at(i);
    }
private:
    Handle Make_handle(const Wall* wall);
    //const Wall* Hovered_wall() const;
    Wall* Hovered_handle();
    Wall* Hovered_wall();
    const Paint* Selected_paint() const;
    Paint* Selected_paint();

    void Add_tool(std::unique_ptr<Tool> tool)
    {
        if (!active_tool) active_tool = tool.get();
        tools.push_back(std::move(tool));
    }
    void Make_tools();
    
    void Edit();
    void Build_paint_menu();
    void Build_tool_menu();
    void Select_handle();
    void Select_paint() noexcept;
    void Paint_surface();
    void Drag_handles();
    void Draw_UI() const;
};

struct Tool_Icon : Menu_Icon
{
    Editor* editor;
    size_t tool_index;

    Tool_Icon(Editor* e, size_t i) noexcept
        : editor(e), tool_index(i) {}

    void Draw(Rectangle rect, bool selected) const override
    {
        editor->Get_tool(tool_index).Draw_swatch(rect);

        if (selected)
            DrawRectangleRoundedLines(rect, 0.5f, 10, 20.0f, DARKGRAY);
    }

    //void On_click() override
    //{
    //    editor->SetActiveTool(tool_index);
    //}
};