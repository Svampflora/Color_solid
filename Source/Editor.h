#pragma once

#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning(disable:ALL_CODE_ANALYSIS_WARNINGS)
#include "raylib.h"
#pragma warning(pop)

#include "settings.h" // move to .cpp
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
    Feature_settings                    feature_settings;
    std::vector<std::unique_ptr<Tool>>  tools;
    Font&                               font;
    Menu                                paint_menu;
    Menu                                tool_menu;
    int                                 active_tool_index;

    Vector3                             room_position = { 0.0f, 0.0f, 0.0f };
    float                               min_size = 1.0f; //TODO: move. settings?
    float                               max_size = 10.0f;

public:

    Editor(Project& project_ref, CameraController& camera_controller_ref, Font& _font);
    std::unique_ptr<State> Update() override;
    Tool& Get_tool(size_t i);
    void Render() const override;

private:

    Wall* Hovered_wall();
    const Paint* Selected_paint() const;
    Paint* Selected_paint();
    void Add_tool(std::unique_ptr<Tool> tool);
    void Select_paint() noexcept;
    void Select_tool(int index);
    void Build_paint_menu();
    void Build_tool_menu();
    void Select_handle();
    void Paint_surface();
    void Make_tools();
    void Edit();
    void Draw_UI() const;
};

struct Tool_Icon : Menu_Icon
{
    Editor* editor;
    size_t tool_index;

    Tool_Icon(Editor* e, size_t i) noexcept :
        editor(e), 
        tool_index(i) 
    {}

    void Draw(Rectangle rect, bool selected, bool hovered, Font& font) const override;


    //void On_click() override
    //{
    //    editor->SetActiveTool(tool_index);
    //}
};