#pragma once
#include "RayUtils.h"
#include "Paint.h"
#include <array>
#include <vector>
#include <functional>


struct Vector3;
struct Color;
struct RayCollision;
struct Ray;

struct Handle
{
    Vector3 last_hit;
    bool selected;


    Handle() noexcept
    {
        last_hit = { 0.0f,0.0f,0.0f };
        selected = false;
    }

    std::function<Vector3()> Position;
    std::function<Vector3()> Normal;

    std::function<void(const Vector3& delta)> on_drag;

    bool Active() const noexcept
    {
        return Position && Normal;
    }

    bool Hovered(const Camera& camera) const;

};

struct Object
{
    Vector3 center;
    float width, height, depth;
    std::vector<Paint*> paint_layers;

    Object() noexcept
    {
        center = {0.0f, 0.0f, 0.0f};
        width = 0.0f;
        height = 0.0f;
        depth = 0.0f;

    }
};

struct Aperture
{
    Vector2 center;
    float width, height, depth;

    Aperture() noexcept
    {
        center = { 0.0f, 0.0f };
        width = 0.0f;
        height = 0.0f;
        depth = 0.0f;
    }
    
    Aperture(const Vector2& _center) noexcept : center{ _center }, width{ 1.0f }, height{ 1.5f }, depth{ 0.1f }
    {};

    virtual ~Aperture() = default;

    std::vector<std::array<Vector3, 4>> Carve(const std::array<Vector3, 4>& main_quad, const std::array<Vector3, 4>& aperture_quad) const;
    std::array<Vector3, 4> Quad(const std::array<Vector3, 4>& wall_quad, const Vector3& wall_normal) const;
    float Area() const noexcept;
    virtual float Height() const noexcept;
    virtual float Width() const noexcept;

    virtual Vector3 Center_position(const std::array<Vector3, 4>& wall_quad) const;
    virtual void Draw(const std::array<Vector3, 4>& wall_quad, const Vector3& wall_normal, const Color& color) const;

};

struct Entrance : public Aperture
{
    float architrave;

    Entrance(const float& _center, const float& wall_height) noexcept;
    virtual ~Entrance() = default;


    std::array<Vector3, 4> Quad(const std::array<Vector3, 4>& w, const Vector3& wall_normal) const;
    Vector3 Center_position(const std::array<Vector3, 4>& wall_quad) const override;

    float Height() const noexcept override;
    float Width() const noexcept  override;
    //TODO:: float Frame_area() const noexcept;
    void Draw(const std::array<Vector3, 4>& wall_quad, const Vector3& wall_normal, const Color& color) const;
};

struct Skirting
{
    float height = 0.15f;
    std::vector<Paint*> paint_layers;

    Color Get_color() const;

    std::vector< std::array<Vector3, 4>>Quads(const std::array<Vector3, 4>& wall_quad, const std::vector<Entrance>& entrances, const Vector3 wall_normal) const;
    float Area(const std::array<Vector3, 4>& wall_quad, const std::vector<Entrance>& entrances, const Vector3& wall_normal) const ;
    bool Is_painted() const noexcept;
    void Add_Paint(Paint& paint);
    void Set_height(const float& new_height) noexcept;
    void Draw(const std::array<Vector3, 4>& wall_quad, const std::vector<Entrance>& entrances, const Vector3& wall_normal, const Color& color) const;
    void Draw_outline(const std::array<Vector3, 4>& wall_quad, const std::vector<Entrance>& entrances, const Vector3& wall_normal, const Color& color) const;


};

struct Wall
{
    std::vector<Paint*> paint_layers;
    std::vector<size_t> vertex_indices;
    const std::vector< Vector3>* room_vertices = nullptr;
    std::vector<Entrance> doors{};
    std::vector<Aperture> windows;

    Skirting skirt_board;

    Wall(const std::vector<size_t>& indices, const std::vector<Vector3>* corners_ptr) noexcept;

    std::vector<std::array<Vector3, 4>> Paint_quads() const;
    std::vector<Vector3> Vertices() const;
    std::array<Vector3, 4> Skirting_quad() const; //TODO: used for mouse-ray intersect only. replace and delete
    std::array<Vector3, 4> Quad() const;
    std::array<Vector3, 3> Triangle() const;
    std::vector<std::array<Vector3, 4>>Cut_bottom(const std::vector<std::array<Vector3, 4>>& quads, float distance_from_bottom) const;
    Vector3 Center() const;
    Vector3 Position(const Vector2& normalized_coordinate) const;
    Vector3 Normal() const;
    Vector3 Floor_edge() const;
    Vector3 Up() const;
    const Vector3& Vertex(size_t i) const;
    float Length() const;
    float Height() const;
    float Total_area() const;
    float Wall_paint_area() const;
    float Liters_of(const Paint* target) const;
    bool Facing_camera(const Vector3 camera_position) const;
    void Add_paint(Paint& paint);
    void Try_add_door();
    void Try_add_aperture() noexcept;
    void Draw_area(const TextAnchor3D anchor) const;
    void Draw_distance(const Vector3& a, const Vector3& b, const Color& color, const TextAnchor3D anchor) const;
    void Draw_outline(const Color color) const;
    void Draw_doors_outline(const Color color) const;
    void Draw_apertures_outline(const Color& color) const;
    void Draw_filled() const;
    void Draw_filled(const Color& default_color) const;
    void Draw() const;
};

RayCollision RayIntersectsWall(const Ray& ray, const Wall& wall);

//RayCollision RayIntersectsSkirting(const Ray& ray, const Wall& wall);

struct Room
{
    Vector3 position{ 0.0f, 1.0f, 0.0f };
    std::vector<Vector3> corners{};
    std::vector<Wall> walls{};
    std::vector<Wall> selected_walls{};
    size_t floor_index;
    size_t cieling_index;

    Room() noexcept;

    float Total_wall_paint_area() const noexcept;
    float Selected_wall_area() const;
    void Generate_box_room(float width, float length, float height) noexcept;
    float Liters_of(const Paint* target) const;
    void Mirror_resize(const Vector3& direction, const Vector3& move_delta);
    void Draw_walls() const;
};