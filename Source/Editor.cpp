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


}

void Editor::Build_tool_menu()
{
    tool_menu = Menu{};

    for (size_t i = 0; i < tools.size(); ++i)
    {
        tool_menu.Add_item(std::make_unique<Tool_Icon>(this, i));
    }
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

void Mirror_resize::Check_hovered()
{

    const Vector2 mouse = GetMousePosition();

    hovered = nullptr;
    for (auto& h : handles)
    {
        if (CheckCollisionPointCircle(mouse, GetWorldToScreen(h.Position(), context.camera), HANDLE_RADIUS))
        {
            hovered = &h;
        }
    }

}

void Mirror_resize::Drag_handles()
{
    if (active)
    {
        const Vector3 wall_normal = active->Normal();
        const Vector3 helper = (fabsf(wall_normal.y) > 0.9f)
            ? Vector3{ 1, 0, 0 } : Vector3{ 0, 1, 0 };

        const Vector3 sideways = Vector3Normalize(Vector3CrossProduct(helper, wall_normal));
        const Vector3 perp_plane_normal = sideways;
        const Ray ray = GetMouseRay(GetMousePosition(), context.camera);
        const float plane_d = Vector3DotProduct(perp_plane_normal, active->Position());
        const RayHit hit = RayIntersectPlane(ray, perp_plane_normal, plane_d);

        if (hit.hit)
        {
            const Vector3 center = active->Position();
            const Vector3 diff = Vector3Subtract(hit.point, center);
            const float distance_along_axis = Vector3DotProduct(diff, wall_normal);
            const Vector3 line_position = Vector3Add(center, Vector3Scale(wall_normal, distance_along_axis));
            const Vector3 move_delta = Vector3Subtract(line_position, active->last_hit);
            active->last_hit = line_position;

            active->on_drag(Vector3Negate(move_delta));
        }
    }
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

    if (tool_menu.Selected_index() != -1)
    {
        tools.at(tool_menu.Selected_index())->Update(camera_controller.camera, project);

    }


    Paint_surface();

    //Drag_handles();

    Alter_skirting();
    
    //if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
    //{
    //    handle.selected = false;
    //}
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

        if (paint_menu.Clicked(mouse_position))
        {
            tool_menu.Deselect();
        }

        if (tool_menu.Clicked(mouse_position))
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


    //for (const auto& wall : project.room.walls)
    //{
    //    const Vector2 screen_position = GetWorldToScreen(wall.Center(), camera_controller.camera);
    //    const float dist_sq = Vector2DistanceSqr(GetMousePosition(), screen_position);

    //    if (dist_sq < closest_distance_sq)
    //    {
    //        closest_distance_sq = dist_sq;
    //        DrawCircleV(screen_position, radius, WHITE);
    //    }
    //    else
    //    {
    //        DrawCircleV(screen_position, 4.0f, GRAY);
    //    }
    //}

    //if (handle.Active())
    //{
    //    if (handle.Hovered(camera_controller.camera) || handle.selected)
    //    {
    //        DrawCircleV(GetWorldToScreen(handle.Position(), camera_controller.camera), radius, PINK);
    //    }
    //}



    paint_menu.Draw(PAINT_MENU_POSITION); 
    tool_menu.Draw(TOOL_MENU_POSITION);
}


void Editor::Alter_skirting()
{
//    Wall* hovered_wall = Hovered_wall();
//
//    if (hovered_wall)
//    {
//         const Ray ray = GetMouseRay(GetMousePosition(), camera_controller.camera);
//         Vector3 handle_pos =  hovered_wall->Center();
//         handle_pos.y = x, hovered_wall->skirt_board.height
//    };
//         const Vector2 screen_position = GetWorldToScreen(wall.Center(), camera_controller.camera);
//
//        if (CheckCollisionPointCircle(GetMousePosition(), handle_pos, 10.0f))
//        {
//
//           hovered_wall->skirt_board.height += GetMouseWheelMove();
//
//
//        }
//
//    }

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
        tools.at(tool_menu.Selected_index())->DrawOverlay();
    }
    if (tool_menu.Selected_index() == 3)
    {
        camera_controller.End_3D();

        tools.at(tool_menu.Selected_index())->DrawOverlay();
    }



    //color_picker.Draw();
    camera_controller.End_3D();

    Draw_UI();

};


void Add_Door::Draw_swatch(Rectangle rect) const noexcept
{
    DrawRectangleRounded(rect, 0.5f, 10, LIGHTGRAY);
    DrawTextF(Name(), rect.x, rect.y, narrow_cast<int>(rect.height), WHITE);
}


void Add_Aperture::Draw_swatch(Rectangle rect) const noexcept
{
    DrawRectangleRounded(rect, 0.5f, 10, LIGHTGRAY);
    DrawTextF(Name(), rect.x, rect.y, narrow_cast<int>(rect.height), WHITE);
}

void Remove::Draw_swatch(Rectangle rect) const noexcept
{
    DrawRectangleRounded(rect, 0.5f, 10, LIGHTGRAY);
    DrawTextF(Name(), rect.x, rect.y, narrow_cast<int>(rect.height), WHITE);
}

void Mirror_resize::Draw_swatch(Rectangle rect) const noexcept
{
    DrawRectangleRounded(rect, 0.5f, 10, LIGHTGRAY);
    DrawTextF(Name(), rect.x, rect.y, narrow_cast<int>(rect.height), WHITE);
}