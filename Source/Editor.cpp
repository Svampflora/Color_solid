#include "Editor.h"

#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning(disable:ALL_CODE_ANALYSIS_WARNINGS)
#include "raymath.h"
#pragma warning(pop)

#include "Utilities.h"
#include "RayUtils.h"
#include "FloorPlanEditor.h"
#include "PaintEditor.h"
#include "Settings.h"

const Vector2 PAINT_MENU_POSITION = { 0.8f * SCREEN_WIDTH, 0.2f * SCREEN_HEIGHT }; //TODO: Settings
const Vector2 TOOL_MENU_POSITION = { 0.1f * SCREEN_WIDTH, 0.2f * SCREEN_HEIGHT };

Editor::Editor(Project& project_ref, CameraController& camRef) :
    project(project_ref),
    camera_controller(camRef),
    paint_menu(),
    feature_settings(),
    tools(),
    active_tool_index{-1},
    tool_menu(),
    font()
{
    camera_controller.Set_birds_eye();
    camera_controller.Set_projection(CAMERA_PERSPECTIVE);

    room_position = project_ref.room.Center();
    camera_controller.Set_target(room_position);

    Make_tools();

    Build_paint_menu();
    Build_tool_menu();

    font = LoadFont("Assets/vcr-osd-mono.ttf");
}

void Editor::Build_paint_menu()
{
    paint_menu = Menu{};

    for (const Paint& p : project.paints)
    {
        paint_menu.Add_item(
            std::make_unique<Paint_Icon>(&p, project.room)
        );
    }
}

void Editor::Make_tools()
{
    Tool_context tool_context{ camera_controller.camera, &project, GetFrameTime() };

    Add_tool(std::make_unique<Add_Door>());
    Add_tool(std::make_unique<Add_Aperture>());
    Add_tool(std::make_unique<Remove>());
    Add_tool(std::make_unique<Mirror_resize>(tool_context));
    Add_tool(std::make_unique<Skirting_resize>(tool_context));


}

void Editor::Build_tool_menu()
{
    tool_menu = Menu{};

    for (size_t i = 0; i < tools.size(); ++i)
    {
        tool_menu.Add_item(std::make_unique<Tool_Icon>(this, i));
    }
}

void Editor::Select_tool(int index)
{

    if (active_tool_index >= 0)
        tools.at(active_tool_index)->On_leave();

    active_tool_index = index;

    tools.at(active_tool_index)->On_enter();
}

Paint* Editor::Selected_paint()
{
    const int i = paint_menu.Selected_index();
    if (i < 0) return nullptr;
    return &project.paints.at(i);
}

const Paint* Editor::Selected_paint() const
{
    const int i = paint_menu.Selected_index();
    if (i < 0) return nullptr;
    return &project.paints.at(i);
}

const Wall* Get_Hovered_wall(const Camera& camera, const std::vector<Wall>& walls)
{
    const Wall* hovered_wall = nullptr;

    for (auto& wall : walls)
    {
        const Ray ray = GetMouseRay(GetMousePosition(), camera);

        if (RayIntersectsWall(ray, wall).hit)
        {
            if (wall.Facing_camera(camera.position))
            {
                hovered_wall = &wall;
            }
        }
    }
    return hovered_wall;
}

Wall* Editor::Hovered_wall()
{
    Wall* hovered_wall = nullptr;

    for (auto& wall : project.room.walls)
    {
        const Ray ray = GetMouseRay(GetMousePosition(), camera_controller.camera);

        if (RayIntersectsWall(ray, wall).hit)
        {
            if (wall.Facing_camera(camera_controller.camera.position))
            {
                hovered_wall = &wall;
            }
        }
    }
    return hovered_wall;
}




void Editor::Edit()
{
    int selected_tool_index = tool_menu.Selected_index();
    if (active_tool_index != selected_tool_index)
    {
        Select_tool(selected_tool_index);
    }

    if (active_tool_index != -1)
    {
        tools.at(active_tool_index)->Update(camera_controller.camera, project);

    }

    Paint_surface();
}


std::unique_ptr<State> Editor::Update()
{
    if (IsKeyReleased(KEY_TAB))
    {
        return std::make_unique<FloorPlanEditor>(project, camera_controller);
    }

    if (IsKeyReleased(KEY_P))
    {
        return std::make_unique<PaintEditor>(project, camera_controller);
    }


    camera_controller.Update();
    Edit();

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        const Vector2 mouse_position = GetMousePosition();

        if (paint_menu.Clicked(PAINT_MENU_POSITION, mouse_position))
        {
            tool_menu.Deselect();
        }

        if (tool_menu.Clicked(TOOL_MENU_POSITION, mouse_position))
        {
            paint_menu.Deselect();
        }
    }

    paint_menu.Update(PAINT_MENU_POSITION); 
    tool_menu.Update(TOOL_MENU_POSITION);   

    return nullptr;
}

void Editor::Draw_UI() const
{
    paint_menu.Draw(PAINT_MENU_POSITION); 
    tool_menu.Draw(TOOL_MENU_POSITION);

    if (tool_menu.Selected_index() != -1)
    {
        tools.at(tool_menu.Selected_index())->Draw_overlay_2D();
    }
}

void Editor::Paint_surface()
{
    Wall* hovered_wall = Hovered_wall();

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && hovered_wall)
    {
        const Ray ray = GetMouseRay(GetMousePosition(), camera_controller.camera);

        Paint* selected_paint = Selected_paint();

        if (selected_paint)
        {
            const RayCollision ray_collision = RayIntersectsQuad(ray, hovered_wall->Skirting_quad());

            if (ray_collision.hit)
            {
                hovered_wall->skirt_board.Add_Paint(*selected_paint); //TODO: return surface area and add to selected paint.area
            }
            else
            {
                hovered_wall->Add_paint(*selected_paint);

            }
        }
    }
}

void Editor::Render() const
{
    camera_controller.Begin_3D();

    project.room.Draw_walls();
    
    const Wall* hovered_wall = Get_Hovered_wall(camera_controller.camera, project.room.walls);
    const Paint* selected_paint = Selected_paint();
    
    if (selected_paint && hovered_wall)
    {
        const Ray ray = GetMouseRay(GetMousePosition(), camera_controller.camera);
    
        const RayCollision ray_collision = RayIntersectsQuad(ray, hovered_wall->Skirting_quad());
    
        if (ray_collision.hit)
        {
            const Color transparent_color = ColorAlpha(selected_paint->color, half_of(1.0f));
            hovered_wall->skirt_board.Draw(hovered_wall->Quad(),hovered_wall->doors,hovered_wall->Normal(), transparent_color);
        }
        else
        {
            const Color transparent_color = ColorAlpha(selected_paint->color, half_of(1.0f));
            hovered_wall->Draw_filled(transparent_color);
        }
    }


    if (tool_menu.Selected_index() != -1)
    {
        tools.at(tool_menu.Selected_index())->Draw_overlay_3D();
    }

    camera_controller.End_3D();

    Draw_UI();

};
