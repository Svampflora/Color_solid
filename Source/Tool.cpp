#include "Tool.h"



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

void Add_Door::DrawOverlay() const
{
    if (!hovered_wall) return;

    Entrance entrance = local_projection(*hovered_wall);

    entrance.Draw(hovered_wall->Quad(), hovered_wall->Normal(), DARKGRAY);
}


void Add_Door::Draw_swatch(Rectangle rect) const noexcept
{
    DrawRectangleRounded(rect, 0.5f, 10, LIGHTGRAY);
    DrawTextF(Name(), rect.x, rect.y, narrow_cast<int>(rect.height), WHITE);
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

void Add_Aperture::DrawOverlay() const
{

    if (!hovered_wall) return;

    Aperture aperture = local_projection(*hovered_wall);

    aperture.Draw(hovered_wall->Quad(), hovered_wall->Normal(), DARKGRAY);
}

void Add_Aperture::Draw_swatch(Rectangle rect) const noexcept
{
    DrawRectangleRounded(rect, 0.5f, 10, LIGHTGRAY);
    DrawTextF(Name(), rect.x, rect.y, narrow_cast<int>(rect.height), WHITE);
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
        Aperture& d = wall.windows[i];

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

void Remove::DrawOverlay() const
{
    if (!hovered.Hit())
        return;
    auto quad = hovered.aperture->Quad(
        hovered.wall->Quad(),
        hovered.wall->Normal());

    DrawQuadLinesEx3D(quad, RED);

    // Draw X
    DrawLine3D(quad[0], quad[2], RED);
    DrawLine3D(quad[1], quad[3], RED);
}

void Remove::Draw_swatch(Rectangle rect) const noexcept
{
    DrawRectangleRounded(rect, 0.5f, 10, LIGHTGRAY);
    DrawTextF(Name(), rect.x, rect.y, narrow_cast<int>(rect.height), WHITE);
}

void Mirror_resize::Update(const Camera& _camera, Project& _project)
{
    context.camera = _camera;
    context.project = &_project;

    Check_hovered();
    if (hovered)
    {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            active = hovered;
            active->last_hit = active->Position();
            hovered->last_hit = hovered->Position();
        }
    }
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
    {
        if (active)
        {
            active = nullptr;
        }

    }
    if (active)
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
        _handle.on_drag = [project, w](auto d) { project->room.Mirror_resize(w.Normal(), d); };
        _handle.last_hit = _handle.Position();

        handles.emplace_back(_handle);
    }
}

void Mirror_resize::DrawOverlay() const
{

    for (auto& h : handles)
    {
        const Vector2 screen = GetWorldToScreen(h.Position(), context.camera);

        Color color = GRAY;

        if (&h == hovered)
            color = PINK;

        if (&h == active)
            color = WHITE;

        DrawCircleV(screen, HANDLE_RADIUS, color);
    }
}

void Mirror_resize::Draw_swatch(Rectangle rect) const noexcept
{
    DrawRectangleRounded(rect, 0.5f, 10, LIGHTGRAY);
    DrawTextF(Name(), rect.x, rect.y, narrow_cast<int>(rect.height), WHITE);
}