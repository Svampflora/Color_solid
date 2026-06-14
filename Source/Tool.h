#pragma once

#include "Utilities.h"
#include "State.h"
#include "Project.h"
#include "CameraController.h"


class Tool
{
public:
    virtual ~Tool() = default;

    virtual const char* Name() const = 0;

    virtual void Update(const Camera& camera, Project& project) = 0;
    virtual void On_enter() noexcept {}
    virtual void On_leave() noexcept {}

    virtual void Draw_overlay_3D() const {}
    virtual void Draw_overlay_2D() const {}
    virtual void Draw_swatch(Rectangle rect, Font& font) const noexcept = 0;
};

class Handled_Tool : public Tool
{
protected:

    std::vector<Handle>     handles;
    std::vector<Handle*>    selected;
    Handle*                 hovered = nullptr;

    void Check_hovered(const Camera&);
    void Draw_handles(const Camera&) const;

    void On_leave() noexcept override
    {
        hovered = nullptr;
        selected.clear();
    }

    bool Is_selected(const Handle* handle) const
    {
        return std::find(
            selected.begin(),
            selected.end(),
            handle)
            != selected.end();
    }
    virtual void Select(Handle* handle)
    {
        if (!handle)
            return;

        if (!Is_selected(handle))
        {
            selected.push_back(handle);
        }
    }
    void Deselect(Handle* handle)
    {
        selected.erase(
            std::remove(
                selected.begin(),
                selected.end(),
                handle),
            selected.end());
    }

    void Select_all()
    {
        selected.clear();
        selected.reserve(handles.size());
        for (auto& h : handles)
        {
            selected.emplace_back(std::addressof(h));
        }
    }
    void Toggle_selection(Handle* handle)
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
    void Clear_selection() noexcept
    {
        selected.clear();
    }
};

class Add_Door : public Tool
{
    Wall* hovered_wall = nullptr;
    Ray ray;

    Entrance local_projection(const Wall wall) const;

public:
    const char* Name() const noexcept override { return "Lägg till Dörr"; }
    void Update(const Camera& camera, Project& project) override;
    void Draw_overlay_3D() const override;
    void Draw_swatch(Rectangle rect, Font& font) const noexcept override;
};

class Add_Aperture : public Tool
{
    Wall* hovered_wall = nullptr;
    Ray ray = {};

    Aperture local_projection(const Wall wall) const;

public:
    const char* Name() const noexcept override { return "Lägg till Fönster"; }
    void Update(const Camera& camera, Project& project) override;
    void Draw_overlay_3D() const override;
    void Draw_swatch(Rectangle rect, Font& font) const noexcept  override;
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

    Aperture_hit Hovered_aperture(Wall& wall, Vector2 local_position);

public:
    const char* Name() const noexcept override { return "Ta bort"; }
    void Update(const Camera& camera, Project& project) override;
    void Draw_overlay_3D() const override;
    void Draw_swatch(Rectangle rect, Font& font) const noexcept override;
};

class Painting : public Tool
{
    //void Paint_surface(const Camera& camera, Project& project)
    //{
    //    const Ray ray = GetMouseRay(GetMousePosition(), camera);

    //    Wall* wall =
    //        project.room.Hovered_wall(camera, ray);

    //    if (!wall)
    //        return;

    //    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && wall)
    //    {
    //        const Ray ray = GetMouseRay(GetMousePosition(), camera);

    //        Paint* selected_paint = project.Selected_paint();

    //        if (selected_paint)
    //        {
    //            const RayCollision ray_collision = RayIntersectsQuad(ray, hovered_wall->Skirting_quad());

    //            if (ray_collision.hit)
    //            {
    //                hovered_wall->skirt_board.Add_Paint(*selected_paint); //TODO: return surface area and add to selected paint.area
    //            }
    //            else
    //            {
    //                hovered_wall->Add_paint(*selected_paint);

    //            }
    //        }
    //    }
    //}

public:
    const char* Name() const noexcept override { return "Måla"; }
    void Update(const Camera& camera, Project& project) override;
    void Draw_overlay_3D() const override;
    void Draw_swatch(Rectangle rect, Font& font) const noexcept override;
};

struct Tool_context
{
    Camera      camera;
    Project*    project;
    float       dt;
};

class Mirror_resize : public Handled_Tool
{
    Tool_context            context;

    Handle* Selected();
    void Select(Handle* handle) override;
public:
    Mirror_resize(Tool_context tool_context) :
    context(tool_context)
    {
        selected.push_back(nullptr);
        Build_handles(context.project);
    }

    void On_leave() noexcept override
    {
        hovered = nullptr;
        selected.at(0) = nullptr;
    }

    const char* Name() const noexcept override { return "Spegel-dra rum"; }
    void Drag_handles();
    void Update(const Camera& _camera, Project& _project) override;
    void Build_handles(Project* project);
    void Draw_overlay_2D() const override;
    void Draw_swatch(Rectangle rect, Font& font) const noexcept override;
};

class Skirting_resize : public Handled_Tool
{
    Tool_context    context;
    Handle*         dragged;

public:
    Skirting_resize (Tool_context tool_context) :
    context(tool_context)
    {
        Build_handles(context.project);
    }

    const char* Name() const noexcept override { return "Dra ut golvlist"; }
    void Update(const Camera& _camera, Project& _project) override;
    void Build_handles(Project* project);
    void Drag_handles();
    void Draw_overlay_2D() const override;
    void Draw_swatch(Rectangle rect, Font& font) const noexcept override;

};