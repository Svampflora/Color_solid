#pragma once
#include "RayUtils.h"
#include "Paint.h"
#include <array>
#include <vector>

struct Vector3;
struct Color;
struct RayCollision;
struct Ray;





struct Aperture
{
    Vector2 center;
    float width, height, depth;

    Aperture() = default;
    Aperture(const Vector2& _center) noexcept : center{ _center }, width{ 1.0f }, height{ 1.5f }, depth{ 0.1f }
    {};

    std::array<Vector3, 4> Quad(const std::array<Vector3, 4>& wall_quad, const Vector3 wall_normal) const;
    Vector3 Center_position(const std::array<Vector3, 4>& wall_quad) const;
    float Area() const noexcept;
    float Height() const noexcept;
    float Width() const noexcept;
    void Draw(const std::array<Vector3, 4>& wall_quad, const Vector3& wall_normal, const Color& color) const;

};

struct Door : public Aperture
{
    float architrave;

    Door(const float& _center, const float& wall_height) noexcept;

    std::array<Vector3, 4> Frame_quad(const std::array<Vector3, 4>& w, const Vector3 wall_normal) const;

    float Frame_height() const noexcept;
    float Frame_width() const noexcept;
    void Draw(const std::array<Vector3, 4>& wall_quad, const Vector3& wall_normal, const Color& color) const;
};

struct Skirting
{
    float height = 0.15f;
    std::vector<Paint*> paint_layers;

    Color Get_color() const;

    bool Is_painted() const noexcept;
    void Add_Paint(Paint& paint);
    void Set_height(const float& new_height) noexcept;
};

struct Wall
{
    struct Handle
    {
        bool hovered;
        bool selected;
        Wall* wall;
        Vector3 last_hit;

        Handle();
    };
    std::vector<Paint*> paint_layers;
    std::vector<size_t> vertex_indices;
    const std::vector< Vector3>* room_vertices = nullptr;
    std::vector<Door> doors{};
    std::vector<Aperture> windows;

    Skirting skirt_board;

    Wall(const std::vector<size_t>& indices, const std::vector<Vector3>* corners_ptr) noexcept;

    std::vector<Vector3> Vertices() const;
    std::array<Vector3, 4> Wall_paint_quad() const;
    std::array<Vector3, 4> Skirting_quad() const;
    std::array<Vector3, 4> Quad() const;
    std::array<Vector3, 3> Triangle() const;
    Vector3 Center() const;
    Vector3 Normal() const;
    Vector3 Floor_edge() const;
    const Vector3& Vertex(size_t i) const;
    float Length() const;
    float Height() const;
    float Total_area() const;
    float Wall_paint_area() const;
    float Skirting_area() const;
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
    void Draw_skirting_outline(const Color color) const;
    void Draw_filled() const;
    void Draw_skirting_filled() const;
    void Draw_skirting_filled(const Color& _color) const;
    void Draw_filled(const Color& default_color) const;
    void Draw() const;
};

RayCollision RayIntersectsWall(const Ray& ray, const Wall& wall);

RayCollision RayIntersectsSkirting(const Ray& ray, const Wall& wall);

struct Room
{
    Vector3 position{ 0.0f, 1.0f, 0.0f };
    std::vector<Vector3> corners{};
    std::vector<Wall> walls{};
    std::vector<Wall> selected_walls{};
    size_t floor_index;
    size_t cieling_index;


    Room() noexcept;

    void Generate_box_room(float width, float length, float height) noexcept;
    float Total_wall_paint_area() const noexcept;
    float Selected_wall_area() const;
    float Liters_of(const Paint* target) const;
    void Mirror_resize(const Wall& dragged_wall, const Vector3& move_delta);
    void Draw_walls() const;
};