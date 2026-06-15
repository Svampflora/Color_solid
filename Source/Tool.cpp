#include "Tool.h"

#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning(disable:ALL_CODE_ANALYSIS_WARNINGS)
#include "raymath.h"
#pragma warning(pop)

#include <iostream>

Entrance Add_Door::local_projection(const Wall wall) const
{
    const RayCollision collision = RayIntersectsWall(ray, wall);
    float local_x = wall.Normalized_coordinate(collision.point).x;
    Entrance preset(local_x, wall.Height()); // TODO: get preset from preset object / feature settings
    const float normalized_width = preset.Width() / wall.Length();
    if (local_x < half_of(normalized_width))
    {
        local_x = half_of(normalized_width);
    }
    else if (local_x > (1 - half_of(normalized_width)))
    {
        local_x = 1 - half_of(normalized_width);
    }

    return Entrance(local_x, wall.Height());
}

void Add_Door::Update(const Camera& camera, Project& project)
{
    hovered_wall = nullptr;

    ray = GetMouseRay(GetMousePosition(), camera);
    hovered_wall = project.room.Hovered_wall(camera, ray);
    if (!hovered_wall) return;

    const RayCollision collision = RayIntersectsWall(ray, *hovered_wall);
    float local_x = hovered_wall->Normalized_coordinate(collision.point).x;

    Entrance entrance = local_projection(*hovered_wall);

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {

        float total_door_width = entrance.Width();
        for (const auto& _door : hovered_wall->doors)
        {
            total_door_width += _door.Width();
        }
        if (total_door_width >= hovered_wall->Length()) //TODO: make function Availible_edge_space(); take mouse position into account
        {
            return;
        }

        hovered_wall->doors.emplace_back(local_x, hovered_wall->Height());
    }
}

void Add_Door::Draw_overlay_3D() const
{
    if (!hovered_wall) return;

    Entrance entrance = local_projection(*hovered_wall);

    entrance.Draw(hovered_wall->Quad(), hovered_wall->Normal(), DARKGRAY);
}


void Add_Door::Draw_swatch(Rectangle rect, Font& font) const noexcept
{
    DrawRectangleRounded(rect, 0.5f, 10, LIGHTGRAY);
    DrawTextEx(font, Name(), { rect.x, rect.y }, rect.height, FONT_SPACING, WHITE);
}


Aperture Add_Aperture::local_projection(const Wall wall) const
{
    const RayCollision collision = RayIntersectsWall(ray, wall);
    Vector2 local_position = wall.Normalized_coordinate(collision.point);
    Aperture preset(wall.Normalized_coordinate(collision.point)); // TODO: get preset from preset object / feature settings
    const Vector2 normalized_dimensions = { preset.Width() / wall.Length(), preset.Height() / wall.Height() };

    if (local_position.x < half_of(normalized_dimensions.x))
    {
        local_position.x = half_of(normalized_dimensions.x);
    }
    else if (local_position.x > (1 - half_of(normalized_dimensions.x)))
    {
        local_position.x = 1 - half_of(normalized_dimensions.x);
    }
    if (local_position.y < half_of(normalized_dimensions.y))
    {
        local_position.y = half_of(normalized_dimensions.y);
    }
    else if (local_position.y > (1 - half_of(normalized_dimensions.y)))
    {
        local_position.y = 1 - half_of(normalized_dimensions.y);
    }

    return Aperture(local_position);
}


void Add_Aperture::Update(const Camera& camera, Project& project)
{
    hovered_wall = nullptr;

    ray = GetMouseRay(GetMousePosition(), camera);
    hovered_wall = project.room.Hovered_wall(camera, ray);
    if (!hovered_wall) return;

    const RayCollision collision = RayIntersectsWall(ray, *hovered_wall);
    Vector2 local_position = hovered_wall->Normalized_coordinate(collision.point);

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        hovered_wall->windows.emplace_back(local_position);
    }
}

void Add_Aperture::Draw_overlay_3D() const
{

    if (!hovered_wall) return;

    Aperture aperture = local_projection(*hovered_wall);

    aperture.Draw(hovered_wall->Quad(), hovered_wall->Normal(), DARKGRAY);
}

void Add_Aperture::Draw_swatch(Rectangle rect, Font& font) const noexcept
{
    DrawRectangleRounded(rect, 0.5f, 10, LIGHTGRAY);
    DrawTextEx(font, Name(), { rect.x, rect.y }, rect.height, FONT_SPACING, WHITE);
}

Remove::Aperture_hit Remove::Hovered_aperture(Wall& wall, Vector2 local_position)
{
    for (size_t i = 0; i < wall.doors.size(); ++i)
    {
        Aperture& d = wall.doors.at(i);


        const Rectangle rec
        {
            d.center.x - half_of(d.Width()) / wall.Length(),
            d.center.y - half_of(d.Height()) / wall.Height(),
            d.Width() / wall.Length(),
            d.Height() / wall.Height()
        };

        if (CheckCollisionPointRec(local_position, rec))
        {
            return Aperture_hit
            {
                &wall,
                &d,
                i,
                Aperture_hit::Type::Door
            };
        }
    }

    for (size_t i = 0; i < wall.windows.size(); ++i)
    {
        Aperture& d = wall.windows.at(i);

        const Rectangle rec
        {
            d.center.x - half_of(d.Width()) / wall.Length(),
            d.center.y - half_of(d.Height()) / wall.Height(),
            d.Width() / wall.Length(),
            d.Height() / wall.Height()
        };

        if (CheckCollisionPointRec(local_position, rec))
        {
            return Aperture_hit
            {
                &wall,
                &d,
                i,
                Aperture_hit::Type::Window
            };
        }
    }

    return Aperture_hit();
}

void Remove::Update(const Camera& camera, Project& project)
{
    const Ray ray = GetMouseRay(GetMousePosition(), camera);

    Wall* wall =
        project.room.Hovered_wall(camera, ray);

    if (!wall)
        return;

    const RayCollision collision =
        RayIntersectsWall(ray, *wall);

    const Vector2 local_position =
        wall->Normalized_coordinate(collision.point);

    hovered = Aperture_hit();

    hovered = Hovered_aperture(*wall, local_position);

    if (hovered.Hit() && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        switch (hovered.type)
        {
        case Aperture_hit::Type::Door:
            hovered.wall->Remove_door(hovered.index);
            break;

        case Aperture_hit::Type::Window:
            hovered.wall->Remove_window(
                hovered.index);
            break;
        }

        hovered = Aperture_hit();
    }
}

void Remove::Draw_overlay_3D() const
{
    if (!hovered.Hit())
        return;
    auto quad = hovered.aperture->Quad(
        hovered.wall->Quad(),
        hovered.wall->Normal());

    DrawQuadLinesEx3D(quad, RED);

    // Draw X
    DrawLine3D(quad.at(0), quad.at(2), RED);
    DrawLine3D(quad.at(1), quad.at(3), RED);
}

void Remove::Draw_swatch(Rectangle rect, Font& font) const noexcept
{
    DrawRectangleRounded(rect, 0.5f, 10, LIGHTGRAY);
    DrawTextEx(font, Name(), { rect.x, rect.y }, rect.height, FONT_SPACING, WHITE);
}

void Handled_Tool::Check_hovered(const Camera& camera)
{
    const Vector2 mouse = GetMousePosition();

    hovered = nullptr;
    for (auto& h : handles)
    {
        if (CheckCollisionPointCircle(mouse, GetWorldToScreen(h.Position(), camera), HANDLE_RADIUS))
        {
            hovered = &h;
        }
    }
}

void Handled_Tool::Draw_handles(const Camera& camera) const
{
    for (auto& h : handles)
    {
        const Vector2 screen = GetWorldToScreen(h.Position(), camera);

        Color color = GRAY;

        if (&h == hovered)
            color = PINK;

        DrawCircleV(screen, HANDLE_RADIUS, color);
    }


    for (auto& h : selected)
    {
        if (h)
        {
            const Vector2 screen = GetWorldToScreen(h->Position(), camera);

            DrawCircleV(screen, HANDLE_RADIUS, WHITE);
        }
    }
}

void Handled_Tool::On_leave() noexcept
{
    hovered = nullptr;
    selected.clear();
}

bool Handled_Tool::Is_selected(const Handle* handle) const
{
    return std::find(
        selected.begin(),
        selected.end(),
        handle)
        != selected.end();
}

void Handled_Tool::Select(Handle* handle)
{
    if (!handle)
        return;

    if (!Is_selected(handle))
    {
        selected.push_back(handle);
    }
}

void Handled_Tool::Deselect(Handle* handle)
{
    selected.erase(
        std::remove(
            selected.begin(),
            selected.end(),
            handle),
        selected.end());
}

void Handled_Tool::Select_all()
{
    selected.clear();
    selected.reserve(handles.size());
    for (auto& h : handles)
    {
        selected.emplace_back(std::addressof(h));
    }
}

void Handled_Tool::Toggle_selection(Handle* handle)
{
    if (!handle)
        return;

    if (Is_selected(handle))
    {
        Deselect(handle);
    }
    else
    {
        Select(handle);
    }
}

void Handled_Tool::Clear_selection() noexcept
{
    selected.clear();
}

Handle* Mirror_resize::Selected()
{
    return selected.at(0);
}

void Mirror_resize::Select(Handle* handle)
{
    selected.at(0) = handle;
}

Mirror_resize::Mirror_resize(Tool_context tool_context) :
    context(tool_context)
{
    selected.push_back(nullptr);
    Build_handles(context.project);
}

void Mirror_resize::On_leave() noexcept
{
    hovered = nullptr;
    selected.at(0) = nullptr;
}

void Mirror_resize::Drag_handles()
{

    if (Selected())
    {
        Handle* active_handle = Selected();

        const Vector3 wall_normal = active_handle->Normal();
        const Vector3 helper = (fabsf(wall_normal.y) > 0.9f)
            ? Vector3{ 1, 0, 0 } : Vector3{ 0, 1, 0 };

        const Vector3 sideways = Vector3Normalize(Vector3CrossProduct(helper, wall_normal));
        const Vector3 perp_plane_normal = sideways;
        const Ray ray = GetMouseRay(GetMousePosition(), context.camera);
        const float plane_d = Vector3DotProduct(perp_plane_normal, active_handle->Position());
        const RayHit hit = RayIntersectPlane(ray, perp_plane_normal, plane_d);

        if (hit.hit)
        {
            const Vector3 center = active_handle->Position();
            const Vector3 diff = Vector3Subtract(hit.point, center);
            const float distance_along_axis = Vector3DotProduct(diff, wall_normal);
            const Vector3 line_position = Vector3Add(center, Vector3Scale(wall_normal, distance_along_axis));
            const Vector3 move_delta = Vector3Subtract(line_position, active_handle->last_hit);
            active_handle->last_hit = line_position;

            active_handle->on_drag(Vector3Negate(move_delta));
        }
    }
}

void Mirror_resize::Update(const Camera& _camera, Project& _project)
{
    context.camera = _camera;
    context.project = &_project;

    Check_hovered(context.camera);
    if (hovered)
    {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            Select(hovered);
        }
    }
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
    {
        if (Selected())
        {
            Select(nullptr);
        }
    }
    if (Selected())
    {
        Drag_handles();

    }
}

void Mirror_resize::Build_handles(Project* project)
{
    handles.reserve(project->room.walls.size());
    for (const Wall& w : project->room.walls)
    {
        Handle _handle{};

        _handle.Position = [w]() { return w.Center(); };
        _handle.Normal = [w]() { return w.Normal(); };
        _handle.Up = [w]() { return w.Up(); };
        _handle.on_drag = [project, w](auto d) { project->room.Mirror_resize(w.Normal(), d); };
        _handle.last_hit = _handle.Position();

        handles.emplace_back(_handle);
    }
}

void Mirror_resize::Draw_overlay_2D() const
{
    std::array<size_t, 2> closest_walls{};
    float closest_distance = Vector3Distance(context.camera.position, context.project->room.Center());
    float second_closest = closest_distance;

    for (size_t i = 0; i < context.project->room.walls.size(); i++)
    {
        if (i == context.project->room.cieling_index || i == context.project->room.floor_index)
            continue;

        const float distance_to_wall = Vector3Distance(context.camera.position, context.project->room.walls.at(i).Center());
        if (distance_to_wall < closest_distance)
        {
            closest_distance = distance_to_wall;
            closest_walls.at(0) = i;

        }
        else if (distance_to_wall < second_closest)
        {
            second_closest = distance_to_wall;
            closest_walls.at(1) = i;

        }
    }

    for (const size_t i : closest_walls)
    {
        if (i == context.project->room.cieling_index || i == context.project->room.floor_index)
            continue;

        DrawDistance2D(context.project->room.walls.at(i).Vertex(0), context.project->room.walls.at(i).Vertex(1), context.camera, WHITE, 26.0f, TextAnchor3D::BottomCenter, context.font);
        DrawDistance2D(context.project->room.walls.at(i).Vertex(0), context.project->room.walls.at(i).Vertex(3), context.camera, WHITE, 26.0f, TextAnchor3D::MiddleRight,  context.font);
    }

    Draw_handles(context.camera);
}

void Mirror_resize::Draw_swatch(Rectangle rect, Font& font) const noexcept
{
    DrawRectangleRounded(rect, 0.5f, 10, LIGHTGRAY);
    DrawTextEx(font, Name(), { rect.x, rect.y }, rect.height, FONT_SPACING, WHITE);
}

Skirting_resize::Skirting_resize(Tool_context tool_context) :
    context(tool_context)
{
    Build_handles(context.project);
}

void Skirting_resize::Update(const Camera& _camera, Project& _project)
{
    context.camera = _camera;
    context.project = &_project;

    Check_hovered(context.camera);

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        if (hovered)
        {
            if (selected.empty())
            {
                Select_all();
            }
            else
            {
                Select(hovered);
            }

            dragged = hovered;
        }
        else
        {
            dragged = nullptr;

        }
    }

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
    {
        Drag_handles();
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
    {
        if (!hovered)
            return;

        for (auto* s : selected)
        {
            s->last_hit = s->Position();
        }
    }

    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
    {
        if (hovered)
        {
            if(selected.size() == handles.size())
            {
                Clear_selection();
                Toggle_selection(hovered);
            }
            else
            {
                Toggle_selection(hovered);
            }
        }
        else
        {
            Clear_selection();
        }
    }
}

void Skirting_resize::Build_handles(Project* project)
{
    handles.clear();

    for (Wall& w : project->room.walls)
    {
        Wall* wall = &w;

        if (w.skirt_board.height > 0)
        {
             Handle _handle{};

             _handle.owner = wall;
             _handle.Position = [ wall]() { return wall->Closest_skirting_position(wall->Center()); };
             _handle.Normal = [wall]() { return wall->Normal(); };
             _handle.Up = [wall]() { return wall->Up(); };
             _handle.on_drag = [project, wall](Vector3 d) { wall->Alter_skirting(d.x) ; };
             _handle.last_hit = _handle.Position();

             handles.push_back(_handle);

        }
    }
}

void Skirting_resize::Drag_handles()
{
    if (!dragged)
        return;

    const Ray ray = GetMouseRay(GetMousePosition(),  context.camera);

    const Vector3 drag_plane_point = dragged->Position();
    const Vector3 drag_plane_normal = dragged->Normal();
    const Vector3 hit = RayIntersectPlane( ray, drag_plane_normal, Vector3Distance(context.camera.position, drag_plane_point)).point;

    const Vector3 delta = Vector3Subtract( hit, dragged->last_hit);
    const float height_delta = Vector3DotProduct( delta, dragged->Up());

    if (Vector3LengthSqr(delta) <= 0.000001f)
        return;

    for (const Handle* h : selected)
    {
        h->on_drag({ height_delta, 0.0f, 0.0f });
    }

    dragged->last_hit = hit;
}

void Skirting_resize::Draw_overlay_2D() const
{
    for (size_t i = 0 ; i < context.project->room.walls.size(); i++)
    {
        if (i == context.project->room.cieling_index || i == context.project->room.floor_index)
            continue;

        std::array<Vector3, 4> c = context.project->room.walls.at(i).Skirting_quad();
        const Vector3 v1_bottom = c.at(0);
        const Vector3 v1_top = c.at(3);

        DrawDistance2D(v1_bottom, v1_top, context.camera, WHITE, 26.0f, TextAnchor3D::MiddleRight, context.font); //TODO move fontsize to Settings
    }
    Draw_handles(context.camera);
}


void Skirting_resize::Draw_swatch(Rectangle rect, Font& font) const noexcept
{
    DrawRectangleRounded(rect, 0.5f, 10, LIGHTGRAY);
    DrawTextEx(font, Name(), { rect.x, rect.y }, rect.height, FONT_SPACING, WHITE);
}


