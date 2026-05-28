#pragma once

#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning(disable:ALL_CODE_ANALYSIS_WARNINGS)
#include "raylib.h"
#pragma warning(pop)

#include "Utilities.h"
#include "State.h"
#include "Project.h"
#include "CameraController.h"

const Wall* Get_Hovered_wall(const Camera& camera, const std::vector<Wall>& walls);

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

    //virtual void OnActivate() {}
    //virtual void OnDeactivate() {}

    virtual void Update(const Camera& camera, Project& project) = 0;
    virtual void DrawOverlay() const = 0;

    virtual void Draw_swatch(Rectangle rect) const noexcept = 0;
};

class Add_Door : public Tool
{

    Wall* hovered_wall = nullptr;
    Ray ray;

    Entrance local_projection(const Wall wall) const
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

public:
    const char* Name() const noexcept override { return "Lägg till Dörr"; }

    void Update(const Camera& camera, Project& project) override
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

    void DrawOverlay() const override
    {
        if (!hovered_wall) return;

        Entrance entrance = local_projection(*hovered_wall);

        entrance.Draw(hovered_wall->Quad(), hovered_wall->Normal(), DARKGRAY);
    }

    void Draw_swatch(Rectangle rect) const noexcept override;
};

class Add_Aperture : public Tool
{
    Wall* hovered_wall = nullptr;
    Ray ray = {};


    Aperture local_projection(const Wall wall) const
    {
        const RayCollision collision = RayIntersectsWall(ray, wall);
        Vector2 local_position = wall.Normalized_coordinate(collision.point);
        Aperture preset(wall.Normalized_coordinate(collision.point)); // TODO: get preset from preset object / feature settings
        const Vector2 normalized_dimensions = { preset.Width() / wall.Length(), preset.Height() / wall.Height()};

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

public:
    const char* Name() const noexcept override { return "Lägg till Fönster"; }

    void Update(const Camera& camera, Project& project) override
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

    void DrawOverlay() const override
    {

        if (!hovered_wall) return;

        Aperture aperture = local_projection(*hovered_wall);

        aperture.Draw(hovered_wall->Quad(), hovered_wall->Normal(), DARKGRAY);
    }

    void Draw_swatch(Rectangle rect) const noexcept override;
};

class Remove : public Tool
{

    struct Aperture_hit
    {
        Wall* wall = nullptr;
        Aperture* aperture = nullptr;
        size_t index = 0;

        enum class Type
        {
            Door,
            Window
        } type = Type::Window;

        bool Hit() const noexcept
        {
            if (aperture == nullptr)
            {
                return false;
            }
            return true;;
        }
    };

    Aperture_hit hovered = {};

    Aperture_hit Hovered_aperture(Wall& wall, Vector2 local_position)
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


public:
    const char* Name() const noexcept override { return "Ta bort"; }

    void Update(const Camera& camera,
        Project& project) override
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

        if (hovered.Hit() &&
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
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

    void DrawOverlay() const override
    {
        if (!hovered.Hit())
            return;
        auto quad =
            hovered.aperture->Quad(
                hovered.wall->Quad(),
                hovered.wall->Normal());

        DrawQuadLinesEx3D(quad, RED);

        // Draw X
        DrawLine3D(quad[0], quad[2], RED);
        DrawLine3D(quad[1], quad[3], RED);
    }

    void Draw_swatch(Rectangle rect) const noexcept override;
};

class Editor : public State
{
    Project&                            project;
    CameraController&                   camera_controller;
    Handle                              handle;
    Menu                                paint_menu;
    Feature_settings                    feature_settings;
    std::vector<std::unique_ptr<Tool>>  tools;
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
    Handle Make_handle(const Wall* wall);
    Wall* Hovered_handle();
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
    void Select_paint() noexcept;
    void Alter_skirting();
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