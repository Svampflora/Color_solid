#include "Editor.h"

#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning(disable:ALL_CODE_ANALYSIS_WARNINGS)
#include "raymath.h"
#pragma warning(pop)

#include "FloorPlanEditor.h"
#include "Utilities.h"
#include "RayUtils.h"

Wall* Editor::Hovered_handle()
{
    const Vector2 mouse = GetMousePosition();
    constexpr float radius = 10.0f;

    Wall* hovered_wall = nullptr;
    float closest_distance_sq = radius * radius;

    for (auto& wall : room.walls)
    {
        const Vector2 screen_position = GetWorldToScreen(wall.Center(), camera_controller.camera);
        const float dist_sq = Vector2DistanceSqr(mouse, screen_position);

        if (dist_sq < closest_distance_sq)
        {
            closest_distance_sq = dist_sq;
            hovered_wall = &wall;
        }
    }
    return hovered_wall;
}



Wall* Editor::Hovered_wall()
{
    Wall* hovered_wall = nullptr;

    for (auto& wall : room.walls)
    {
        const Ray ray = GetMouseRay(GetMousePosition(), camera_controller.camera);

        if (RayIntersectsWall(ray, wall).hit)
        {
            if (wall.Facing_camera(camera_controller.camera.position)) //TODO: create .camera_position()
            {
                hovered_wall = &wall;
            }
        }
    }
    return hovered_wall;
}

const Wall* Editor::Hovered_wall() const
{
    const Wall* hovered_wall = nullptr;

    for (auto& wall : room.walls)
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

    Wall* hovered_wall = Hovered_wall();

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && hovered_wall)
    {
        const Ray ray = GetMouseRay(GetMousePosition(), camera_controller.camera);

        if (selected_paint)
        {
            if (RayIntersectsSkirting(ray, *hovered_wall).hit)
            {
                hovered_wall->skirt_board.Add_Paint(*selected_paint);
            }
            else
            {
                hovered_wall->Add_Paint(*selected_paint);

            }
        }
    }


    Wall* hovered_handle = Hovered_handle();
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && hovered_handle)
    {
        handle.wall = hovered_handle;
        handle.last_hit = hovered_handle->Center();
        handle.selected = true;
    }
    if (handle.selected && handle.wall)
    {
        const Vector3 wall_normal = handle.wall->Normal();
        const Vector3 helper = (fabsf(wall_normal.y) > 0.9f)
            ? Vector3{ 1, 0, 0 } : Vector3{ 0, 1, 0 };

        const Vector3 sideways = Vector3Normalize(Vector3CrossProduct(helper, wall_normal));
        const Vector3 perp_plane_normal = sideways;
        const Ray ray = GetMouseRay(GetMousePosition(), camera_controller.camera);
        const float plane_d = Vector3DotProduct(perp_plane_normal, handle.wall->Center());
        const RayHit hit = RayIntersectPlane(ray, perp_plane_normal, plane_d);

        if (hit.hit)
        {
            const Vector3 center = handle.wall->Center();
            const Vector3 diff = Vector3Subtract(hit.point, center);
            const float t = Vector3DotProduct(diff, wall_normal); // distance along axis
            const Vector3 line_position = Vector3Add(center, Vector3Scale(wall_normal, t));
            const Vector3 move_delta = Vector3Subtract(line_position, handle.last_hit);
            handle.last_hit = line_position;

            room.Mirror_resize(*handle.wall, Vector3Negate(move_delta));
        }
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
    {
        handle = Wall::Handle{};
    }
}

void Editor::Paint_selection() noexcept
{
    const Rectangle paint_menu{ 0.8f * GetScreenWidthF(), 0.2f * GetScreenHeightF(), 80, 80 };

    //selected_paint = paint_menu(paints);

    for (auto& paint : paints)
    {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            if (CheckCollisionPointRec(GetMousePosition(), paint_menu))
            {
                selected_paint = &paint;
            }
            else
            {
                selected_paint = nullptr;

            }
        }
    }
}

Editor::Editor(Room& roomRef, CameraController& camRef) : 
    room(roomRef), 
    camera_controller(camRef)
{
    
    paints.push_back(Paint());
    camera_controller.Set_birds_eye();
}

std::unique_ptr<State> Editor::Update()
{
    if (IsKeyReleased(KEY_TAB))
    {
        return std::make_unique<FloorPlanEditor>(room, camera_controller);
    }

    camera_controller.Update();
    Edit();
    Paint_selection();

    return nullptr;
}

void Editor::Draw_UI() const 
{
    constexpr float radius = 10.0f;
    float closest_distance_sq = radius * radius;

    for (const auto& wall : room.walls)
    {
        const Vector2 screen_position = GetWorldToScreen(wall.Center(), camera_controller.camera);
        const float dist_sq = Vector2DistanceSqr(GetMousePosition(), screen_position);

        if (dist_sq < closest_distance_sq)
        {
            closest_distance_sq = dist_sq;
            DrawCircleV(screen_position, radius, WHITE);
        }
        else
        {
            DrawCircleV(screen_position, 4.0f, GRAY);
        }
    }

    const Rectangle paint_menu{ 0.8f * GetScreenWidthF(), 0.2f * GetScreenHeightF(), 80, 80 };
    for (const auto& paint : paints)
    {
        DrawRectangleRounded(paint_menu, 0.5f, 10, paint.color);
        const float liters = room.Liters_of(&paint);
        DrawTextF(TextFormat("%.1f L", liters), paint_menu.x + paint_menu.width, paint_menu.y, 25, RAYWHITE);
        DrawTextF(TextFormat("%i strykningar", paint.coats), paint_menu.x + paint_menu.width, paint_menu.y + 30, 25, RAYWHITE);

    }
    if (selected_paint)
    {
        DrawRectangleRoundedLines(paint_menu, 0.5f, 10, 20.0f, DARKGRAY);

    }
}

void Editor::Render() const
{
    camera_controller.Begin_3D();

    room.Draw_walls();

    const Wall* hovered_wall = Hovered_wall();
    if (selected_paint && hovered_wall)
    {
        const Ray ray = GetMouseRay(GetMousePosition(), camera_controller.camera);

        if (RayIntersectsSkirting(ray, *hovered_wall).hit)
        {
            const Color transparent_color = ColorAlpha(selected_paint->color, half_of(1.0f));
            hovered_wall->Draw_skirting_filled(transparent_color);
        }
        else
        {
            const Color transparent_color = ColorAlpha(selected_paint->color, half_of(1.0f));
            hovered_wall->Draw_filled(transparent_color);
        }

    }

    camera_controller.End_3D();

    Draw_UI();

};
