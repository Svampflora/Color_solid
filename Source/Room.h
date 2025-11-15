#pragma once
#include "RayUtils.h"
#include "Paint.h"
#include <array>
#include <vector>

struct Vector3;
struct Color;
struct RayCollision;
struct Ray;




struct Door
{
    float wall_location; //TODO: only stored here for convinience; door does not need to know where it is
    float width, height, architrave;

    Door(const float& _wall_location) noexcept;

    std::array<Vector3, 4> Quad(const Vector3& wall_normal, const Vector3& bottom_center) const;
    std::array<Vector3, 4> Frame_quad(const Vector3& wall_normal, const Vector3& bottom_center) const;
    float Area() const noexcept;
    float Height() const noexcept;
    float Width() const noexcept;
    float Frame_height() const noexcept;
    float Frame_width() const noexcept;

};

struct Aperture
{
    Vector2 center;
    float width, height, depth;

    Aperture() : center{ 0.5f, 0.5f }, width{ 1.0f }, height{ 1.5f }, depth{ 0.1f }
    {};

    std::array<Vector3, 4> Quad(const std::array<Vector3, 4>& wall_quad, const Vector3 wall_normal) const;


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
    //std::vector<Vector3> Wall_paint_polygon() const
    //{
    //    std::array<Vector3, 4> skirting_quad = Skirting_quad();
    //    std::array<Vector3, 4> wall_quad = Quad();
    //    wall_quad.at(0) = skirting_quad.at(3);
    //    wall_quad.at(1) = skirting_quad.at(2);
    //    return wall_quad;
    //}
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
    void Add_Paint(Paint& paint);
    void Try_Add_Door() noexcept;
    void Try_add_Aperture() noexcept;
    void Draw_Area(const TextAnchor3D anchor) const;
    void Draw_Distance(const Vector3& a, const Vector3& b, const Color& color, const TextAnchor3D anchor) const;
    void Draw_outline(const Color color) const;
    void Draw_doors_outline(const Color color) const;
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