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

    //virtual void On_activate() {}
    //virtual void On_deactivate() {}

    virtual void Update(const Camera& camera, Project& project) = 0;
    virtual void DrawOverlay() const = 0;

    virtual void Draw_swatch(Rectangle rect) const noexcept = 0;
};

class Add_Door : public Tool
{
    Wall* hovered_wall = nullptr;
    Ray ray;

    Entrance local_projection(const Wall wall) const;

public:
    const char* Name() const noexcept override { return "Lägg till Dörr"; }
    void Update(const Camera& camera, Project& project) override;
    void DrawOverlay() const override;
    void Draw_swatch(Rectangle rect) const noexcept override;
};

class Add_Aperture : public Tool
{
    Wall* hovered_wall = nullptr;
    Ray ray = {};

    Aperture local_projection(const Wall wall) const;

public:
    const char* Name() const noexcept override { return "Lägg till Fönster"; }
    void Update(const Camera& camera, Project& project) override;
    void DrawOverlay() const override;
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

    Aperture_hit Hovered_aperture(Wall& wall, Vector2 local_position);

public:
    const char* Name() const noexcept override { return "Ta bort"; }
    void Update(const Camera& camera, Project& project) override;
    void DrawOverlay() const override;
    void Draw_swatch(Rectangle rect) const noexcept override;
};

struct Tool_context
{
    Camera      camera;
    Project*    project;
    float       dt;
};

class Mirror_resize : public Tool
{
    Tool_context            context;
    std::vector<Handle>     handles;
    Handle*                 hovered;
    Handle*                 active;

    void Check_hovered();
    void Drag_handles();


public:
    Mirror_resize(Tool_context tool_context) :
    context(tool_context)
    {
        Build_handles(context.project);
    }

    const char* Name() const noexcept override { return "Dra ut rum"; }
    void Update(const Camera& _camera, Project& _project) override;
    void Build_handles(Project* project);
    void DrawOverlay() const override;
    void Draw_swatch(Rectangle rect) const noexcept override;

};