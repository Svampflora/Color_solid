#include "Editor.h"

#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning(disable:ALL_CODE_ANALYSIS_WARNINGS)
#include "raymath.h"
#pragma warning(pop)

#include "FloorPlanEditor.h"
#include "Utilities.h"
#include "RayUtils.h"



Editor::Editor(Room& roomRef, CameraController& camRef) :
    room(roomRef),
    camera_controller(camRef)
{

    paints.push_back(Paint({ 250, 150, 150, 255 }));
    paints.push_back(Paint({ 237, 237, 213, 255 }));
    paints.push_back(Paint({ 66, 95, 150, 255 }));

    Build_paint_menu();
    camera_controller.Set_birds_eye();
    font = LoadFont("Assets/vcr-osd-mono.ttf");
}

void Editor::Build_paint_menu()
{
    paint_menu = Menu{};

    for (const Paint& p : paints)
    {
        paint_menu.Add_item(
            std::make_unique<Paint_Icon>(&p, room)
        );
    }
}

Paint* Editor::Selected_paint()
{
    const int i = paint_menu.Selected_index();
    if (i < 0) return nullptr;
    return &paints.at(i);
}

const Paint* Editor::Selected_paint() const
{
    const int i = paint_menu.Selected_index();
    if (i < 0) return nullptr;
    return &paints.at(i);
}

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

    Paint_surface();

    Drag_handles();
    
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
    {
        handle.selected = false;
    }
}


Handle Editor::Make_handle(const Wall* w)
{
    Handle _handle{};

    _handle.Position = [w]() { return w->Center(); };
    _handle.Normal = [w]() { return w->Normal(); };
    _handle.on_drag = [this,w](auto d) { room.Mirror_resize(w->Normal(), d); };
    _handle.last_hit = _handle.Position();

    return _handle;
}


std::unique_ptr<State> Editor::Update()
{
    if (IsKeyReleased(KEY_TAB))
    {
        return std::make_unique<FloorPlanEditor>(room, camera_controller);
    }

    camera_controller.Update();
    Edit();
    paint_menu.Update({ 0.8f * GetScreenWidthF(), 0.2f * GetScreenHeightF() }); //TODO: repeated magic menu-position

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

    if (handle.Active())
    {
        if (handle.Hovered(camera_controller.camera) || handle.selected)
        {
            DrawCircleV(GetWorldToScreen(handle.Position(), camera_controller.camera), radius, PINK);

        }

    }

    paint_menu.Draw({ 0.8f * GetScreenWidthF(), 0.2f * GetScreenHeightF() }); //TODO: repeated magic menu-position
}

void Editor::Drag_handles()
{

    if (const Wall* w = Hovered_handle())
    {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            handle = Make_handle(w);
            handle.selected = true;
        }
    }

    
    if (handle.selected)
    {
        const Vector3 wall_normal = handle.Normal();
        const Vector3 helper = (fabsf(wall_normal.y) > 0.9f)
            ? Vector3{ 1, 0, 0 } : Vector3{ 0, 1, 0 };

        const Vector3 sideways = Vector3Normalize(Vector3CrossProduct(helper, wall_normal));
        const Vector3 perp_plane_normal = sideways;
        const Ray ray = GetMouseRay(GetMousePosition(), camera_controller.camera);
        const float plane_d = Vector3DotProduct(perp_plane_normal, handle.Position());
        const RayHit hit = RayIntersectPlane(ray, perp_plane_normal, plane_d);

        if (hit.hit)
        {
            const Vector3 center = handle.Position();
            const Vector3 diff = Vector3Subtract(hit.point, center);
            const float distance_along_axis = Vector3DotProduct(diff, wall_normal); 
            const Vector3 line_position = Vector3Add(center, Vector3Scale(wall_normal, distance_along_axis));
            const Vector3 move_delta = Vector3Subtract(line_position, handle.last_hit);
            handle.last_hit = line_position;

            handle.on_drag(Vector3Negate(move_delta));
        }
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

    room.Draw_walls();

    const Wall* hovered_wall = Hovered_wall();
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

    camera_controller.End_3D();

    Draw_UI();

};
