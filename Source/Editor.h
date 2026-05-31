#pragma once

#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning(disable:ALL_CODE_ANALYSIS_WARNINGS)
#include "raylib.h"
#pragma warning(pop)


#include "Tool.h"


const Wall* Get_Hovered_wall(const Camera& camera, const std::vector<Wall>& walls);

struct Camera3D;

struct Feature_settings
{
    Aperture aperture{};
    Aperture entrance{};

};



class Editor : public State
{
    Project&                            project;
    CameraController&                   camera_controller;
    Menu                                paint_menu;
    Feature_settings                    feature_settings;
    std::vector<std::unique_ptr<Tool>>  tools;
    int                                 active_tool_index;
    Menu                                tool_menu;
    Font                                font;

    Vector3                             room_position = { 0.0f, 0.0f, 0.0f };
    float                               min_size = 1.0f; //TODO: move. settings?
    float                               max_size = 10.0f;

public:

    Editor(Project& project_ref, CameraController& camera_controller_ref);
    std::unique_ptr<State> Update() override;
    void Render() const override;

    Tool& Get_tool(size_t i)
    {
        return *tools.at(i);
    }
private:
    //Handle Make_handle(const Wall* wall);
    Wall* Hovered_wall();
    const Paint* Selected_paint() const;
    Paint* Selected_paint();

    void Add_tool(std::unique_ptr<Tool> tool)
    {
        tools.push_back(std::move(tool));
    }
    void Make_tools();
    
    void Edit();
    void Build_paint_menu();
    void Build_tool_menu();
    void Select_handle();
    void Select_tool(int index);
    void Select_paint() noexcept;
    void Paint_surface();
    void Draw_UI() const;
};

struct Tool_Icon : Menu_Icon
{
    Editor* editor;
    size_t tool_index;

    Tool_Icon(Editor* e, size_t i) noexcept
        : editor(e), tool_index(i) {}

    void Draw(Rectangle rect, bool selected, bool hovered) const override
    {
        editor->Get_tool(tool_index).Draw_swatch(rect);

        if (hovered)
            DrawRectangleRoundedLines(rect, 0.5f, 10, 20.0f, DARKGRAY);

        if (selected)
            DrawRectangleRoundedLines(rect, 0.5f, 10, 20.0f, GRAY);
    }

    //void On_click() override
    //{
    //    editor->SetActiveTool(tool_index);
    //}
};