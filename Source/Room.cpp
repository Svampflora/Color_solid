#include "Room.h"

#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning(disable:ALL_CODE_ANALYSIS_WARNINGS)
#include "raymath.h"
#pragma warning(pop)

#include "RayUtils.h"
#include "Utilities.h"
#include <format>

Door::Door(const float& _wall_location) noexcept
{
    width = 1.0f,
        height = 2.0f,
        architrave = 0.05f;
    wall_location = _wall_location;
}

std::array<Vector3, 4> Door::Quad(const Vector3& wall_normal, const Vector3& bottom_center) const
{
    constexpr Vector3 world_up = { 0.0f, 1.0f, 0.0f };

    const Vector3 right = Vector3Normalize(Vector3CrossProduct(world_up, wall_normal));
    const Vector3 up = Vector3Normalize(world_up);

    const Vector3 p0 = Vector3Subtract(bottom_center, Vector3Scale(right, half_of(width)));     // bottom left
    const Vector3 p1 = Vector3Add(bottom_center, Vector3Scale(right, half_of(width)));           // bottom right
    const Vector3 p2 = Vector3Add(p1, Vector3Scale(up, height));                               // top right
    const Vector3 p3 = Vector3Add(p0, Vector3Scale(up, height));                               // top left

    return{ p0, p1, p2, p3 };
}

std::array<Vector3, 4> Door::Frame_quad(const Vector3& wall_normal, const Vector3& bottom_center) const
{
    constexpr Vector3 world_up = { 0.0f, 1.0f, 0.0f };

    const Vector3 right = Vector3Normalize(Vector3CrossProduct(world_up, wall_normal));
    const Vector3 up = Vector3Normalize(world_up);

    const Vector3 p0 = Vector3Subtract(bottom_center, Vector3Scale(right, half_of(Frame_width())));     // bottom left
    const Vector3 p1 = Vector3Add(bottom_center, Vector3Scale(right, half_of(Frame_width())));           // bottom right
    const Vector3 p2 = Vector3Add(p1, Vector3Scale(up, Frame_height()));                               // top right
    const Vector3 p3 = Vector3Add(p0, Vector3Scale(up, Frame_height()));                               // top left

    return{ p0, p1, p2, p3 };
}

float Door::Area() const noexcept
{
    return width * height;
}

float Door::Height() const noexcept
{
    return height;
}

float Door::Width() const noexcept
{
    return width;
}

float Door::Frame_height() const noexcept
{
    return height + architrave;
}

float Door::Frame_width() const noexcept
{
    return width + architrave * 2;
}

//Window::Window()
//{
//}
//
//float Window::Area() const noexcept
//{
//    return width * height;
//}

Color Skirting::Get_color() const 
{
    if (paint_layers.empty())
    {
        throw std::runtime_error("skirting has no color to provide");
    }
    return paint_layers.front()->color;
}

bool Skirting::Is_painted() const noexcept
{
    return !paint_layers.empty();
}

void Skirting::Add_Paint(Paint& paint)
{
    paint_layers.push_back(&paint);
}

void Skirting::Set_height(const float& new_height) noexcept
{
    height = new_height;
}

Wall::Wall(const std::vector<size_t>& indices, const std::vector<Vector3>* corners_ptr) noexcept :
    vertex_indices(indices),
    room_vertices(corners_ptr)
{}

std::vector<Vector3> Wall::Vertices() const
{
    if (!room_vertices || room_vertices->size() < 3)
        throw std::runtime_error("Wall::Vertices() called with null or too few corners points.");

    std::vector<Vector3> points;
    points.reserve(vertex_indices.size());

    for (size_t i = 0; i < vertex_indices.size(); ++i)
    {
        points.emplace_back(Vertex(i));
    }
    return points;
}

std::array<Vector3, 4> Wall::Wall_paint_quad() const
{
    if (skirt_board.height <= 0.0f)
    {
        return Quad();
    }
    std::array<Vector3, 4> skirting_quad = Skirting_quad();
    std::array<Vector3, 4> wall_quad = Quad();
    wall_quad.at(0) = skirting_quad.at(3);
    wall_quad.at(1) = skirting_quad.at(2);

    return wall_quad;
}

std::array<Vector3, 4> Wall::Skirting_quad() const
{
    if (room_vertices->size() < 4) throw;

    const Vector3 v0_bottom = Vertex(0);
    const Vector3 v1_bottom = Vertex(1);
    const Vector3 v1_top = Vertex(2);
    const Vector3 v0_top = Vertex(3); //TODO: should be the last index of wall for polygon-support
    const float plane_y = v0_bottom.y + skirt_board.height;

    Vector3 v0_skirting, v1_skirting;
    IntersectLineWithHorizontalPlane(v0_bottom, v0_top, plane_y, v0_skirting);
    IntersectLineWithHorizontalPlane(v1_bottom, v1_top, plane_y, v1_skirting);

    return { v0_bottom, v1_bottom, v1_skirting, v0_skirting };
}

std::array<Vector3, 4> Wall::Quad() const //TODO: replace with std::vector Polygon()
{
    if (room_vertices->size() < 4) throw std::runtime_error("too few vertices availible");

    const Vector3 p0 = Vertex(0);
    const Vector3 p1 = Vertex(1);
    const Vector3 p2 = Vertex(2);
    const Vector3 p3 = Vertex(3);

    return{ p0, p1, p2, p3 };
}

std::array<Vector3, 3> Wall::Triangle() const
{
    if (room_vertices->size() < 3) throw std::runtime_error("too few vertices availible");


    const Vector3 p0 = Vertex(0);
    const Vector3 p1 = Vertex(1);
    const Vector3 p2 = Vertex(2);

    return{ p0, p1, p2 };
}

Vector3 Wall::Center() const
{
    if (room_vertices->empty()) return Vector3Zero();

    Vector3 sum = Vector3Zero();
    for (size_t i = 0; i < vertex_indices.size(); ++i)
    {
        sum = Vector3Add(sum, Vertex(i));
    }

    return Vector3Scale(sum, 1.0f / static_cast<float>(vertex_indices.size()));
}

Vector3 Wall::Normal() const
{
    if (room_vertices->size() < 3) return Vector3Zero();
    std::array<Vector3, 4> arr = Quad();
    std::vector <Vector3> vec(arr.begin(), arr.end());
    return PolygonNormal(vec);
}

Vector3 Wall::Floor_edge() const
{
    return Vector3Subtract(Vertex(1), Vertex(0));
}

const Vector3& Wall::Vertex(size_t i) const
{
    if (!room_vertices) {
        throw std::runtime_error("room_corners pointer is null");
    }
    if (i >= vertex_indices.size()) {
        throw std::out_of_range("Corner index out of bounds in Wall");
    }
    const size_t corner_i = vertex_indices.at(i);
    if (corner_i >= room_vertices->size()) {
        throw std::out_of_range("Wall::corner_indices contains invalid index");
    }

    return (*room_vertices).at(vertex_indices.at(i));
}

float Wall::Length() const
{
    if (room_vertices->size() < 2) return 0.0f;

    Vector3 lowest1 = Vertex(0);
    Vector3 lowest2 = Vertex(1);

    if (lowest2.y < lowest1.y)
        std::swap(lowest1, lowest2);

    for (size_t i = 0; i < vertex_indices.size(); i++)
    {

        if (Vertex(i).y < lowest1.y)
        {
            lowest2 = lowest1;
            lowest1 = Vertex(i);
        }
        else if (Vertex(i).y < lowest2.y)
        {
            lowest2 = Vertex(i);
        }
    }

    constexpr float tolerance = 0.001f;
    if (std::abs(lowest1.y - lowest2.y) > tolerance)
        return 0.0f;

    return Vector2Distance({ lowest1.x, lowest1.z }, { lowest2.x, lowest2.z });
}

float Wall::Height() const
{
    if (room_vertices->size() < 2) return 0.0f;

    float min_y = Vertex(0).y;
    float max_y = Vertex(0).y;

    for (const auto& c : *room_vertices)
    {
        if (c.y < min_y) min_y = c.y;
        if (c.y > max_y) max_y = c.y;
    }

    return max_y - min_y;
}

float Wall::Total_area() const
{
    return PolygonArea(Vertices());

}

float Wall::Wall_paint_area() const
{
    std::array<Vector3, 4> arr = Wall_paint_quad();
    std::vector <Vector3> vec(arr.begin(), arr.end());
    return PolygonArea(vec);
}

float Wall::Skirting_area() const
{
    return Total_area() - Wall_paint_area();
}

float Wall::Liters_of(const Paint* target) const
{
    float total = 0.0f;

    for (const auto& layer : paint_layers)
    {
        if (layer == target)
        {
            const size_t coats = layer->coats > 0 ? layer->coats : layer->coats;
            total += (Wall_paint_area() * coats) / layer->m2_per_liter;
        }
    }

    for (const auto& layer : skirt_board.paint_layers)
    {
        if (layer == target)
        {
            const size_t coats = layer->coats > 0 ? layer->coats : layer->coats;
            total += (Skirting_area() * coats) / layer->m2_per_liter;
        }
    }

    return total;
}

bool Wall::Facing_camera(const Vector3 camera_position) const
{
    const Vector3 to_camera = Vector3Normalize(Vector3Subtract(camera_position, Center()));
    const float dot = Vector3DotProduct(to_camera, Normal());
    return dot > 0.0f;
}

void Wall::Add_Paint(Paint& paint)
{
    paint_layers.push_back(&paint);
}

void Wall::Try_Add_Door() noexcept
{
    float total_door_width = 1.0f; //TODO: magic prediction of door.width
    for (const auto& door : doors)
    {
        total_door_width += door.Frame_width();
    }
    if (total_door_width >= Length()) //TODO: Floor_edge.magnitude == Length()?
    {
        return;
    }


    //const Vector3 floor_direction = Vector3Normalize(Floor_edge());

    const Door door(0.5f);
    //TODO: squeeze in door where possible
    doors.push_back(door);
}

void Wall::Draw_Area(const TextAnchor3D anchor) const
{
    const Vector3 forward = Normal();
    const Vector3 world_up = { 0.0f, 1.0f, 0.0f };
    const Vector3 right = Vector3Normalize(Vector3CrossProduct(world_up, forward));
    const Vector3 up = Vector3Normalize(Vector3CrossProduct(forward, right));

    //DrawLine3D(Center(), Vector3Add(Center(), Vector3Scale(right, 1.0f)), RED);
    //DrawLine3D(Center(), Vector3Add(Center(), Vector3Scale(up, 1.0f)), BLUE);
    //DrawLine3D(Center(), Vector3Add(Center(), Vector3Scale(forward, 1.0f)), GREEN);

    const Matrix rotation =
    {
        right.x, up.x, forward.x, 0,
        right.y, up.y, forward.y, 0,
        right.z, up.z, forward.z, 0,
        0,       0,    0,         1
    };

    const Vector3 local_pos = { 0.0f, 0.01f, 0.0f }; // on wall, slightly above center, facing out
    const Vector3 world_pos = Vector3Add(Center(), local_pos);

    DrawAnchoredText3D
    (
        GetFontDefault(),
        TextFormat("%.1f M2", Wall_paint_area()),
        world_pos,
        0.4f, 0.1f,
        false,
        WHITE,
        anchor,
        rotation
    );
}

void Wall::Draw_Distance(const Vector3& a, const Vector3& b, const Color& color, const TextAnchor3D anchor) const
{
    //DrawLine3D(a, b, color);
    const Vector3 mid_point = Vector3Scale(Vector3Add(a, b), 0.5f);
    const float distance = Vector3Distance(a, b);

    const Vector3 forward = Vector3Negate(Normal());
    const Vector3 world_up = { 0.0f, 1.0f, 0.0f };
    const Vector3 right = Vector3Normalize(Vector3CrossProduct(world_up, forward));
    const Vector3 up = Vector3Normalize(Vector3CrossProduct(forward, right));


    const Matrix rotation =
    {
        right.x, up.x, forward.x, 0,
        right.y, up.y, forward.y, 0,
        right.z, up.z, forward.z, 0,
        0,       0,    0,         1
    };

    DrawAnchoredText3D
    (
        GetFontDefault(),
        TextFormat("%.2f M", distance),
        mid_point,
        0.2f, 0.1f,
        false,
        color,
        anchor,
        rotation
    );
}

void Wall::Draw_outline(const Color color) const
{


    DrawPolygonLinesEx3D(Vertices(), color);

    //Draw_Area(TextAnchor3D::Center);
    //Draw_Distance(Vertex(0), Vertex(1), color, TextAnchor3D::TopCenter);
    //Draw_Distance(Vertex(0), Vertex(3), color, TextAnchor3D::MiddleLeft);


    //std::array<Vector3, 4> arr = Skirting_quad();
    //std::vector <Vector3> vec(arr.begin(), arr.end());
    //DrawPolygonLinesEx3D(vec, skirt_board.Get_color());

}

void Wall::Draw_doors_outline(const Color color) const
{
    if (doors.empty())
    {
        return;
    }

    const Vector3 floor_direction = Vector3Normalize(Floor_edge());
    for (int i = 0; i < doors.size(); i++)
    {
        const float door_location = 1.0f / (i + 2);
        const float corner_to_door = door_location * Length();
        const Vector3 door_position = Vector3Add(Vertex(0), Vector3Scale(floor_direction, corner_to_door));

        DrawQuadLinesEx3D(doors.at(i).Frame_quad(Normal(), door_position), color);
        DrawQuadLinesEx3D(doors.at(i).Quad(Normal(), door_position), color);
    }
}

void Wall::Draw_skirting_outline(const Color color) const
{

    std::array<Vector3, 4> arr = Skirting_quad();
    std::vector <Vector3> vec(arr.begin(), arr.end());
    DrawPolygonLinesEx3D(vec, color);

}

void Wall::Draw_filled() const
{
    DrawQuad(Wall_paint_quad(), paint_layers.front()->color);
}

void Wall::Draw_skirting_filled() const
{
    DrawQuad(Skirting_quad(), skirt_board.Get_color());

}

void Wall::Draw_skirting_filled(const Color& _color) const
{
    DrawQuad(Skirting_quad(), _color);

}

void Wall::Draw_filled(const Color& default_color) const
{
    if (paint_layers.empty())
    {
        DrawQuad(Wall_paint_quad(), default_color);
    }
    else
    {
        Draw_filled();
    }
}

void Wall::Draw() const
{
    const Color text_color = WHITE;

    if (!paint_layers.empty())
    {
        Draw_filled();
    }

    Draw_outline(WHITE);
    Draw_Area(TextAnchor3D::Center);
    Draw_Distance(Vertex(0), Vertex(1), text_color, TextAnchor3D::TopCenter);
    Draw_Distance(Vertex(0), Vertex(3), text_color, TextAnchor3D::MiddleLeft);

    if (skirt_board.Is_painted())
    {
        Draw_skirting_filled();
    }
    else
    {
        Draw_skirting_outline(WHITE);
    }

    if (!doors.empty())
    {
        Draw_doors_outline(WHITE);
    }
}

RayCollision RayIntersectsWall(const Ray& ray, const Wall& wall)
{
    const std::array<Vector3, 4> quad = wall.Quad();

    const Vector3 p0 = quad.at(0);
    const Vector3 p1 = quad.at(1);
    const Vector3 p2 = quad.at(2);
    const Vector3 p3 = quad.at(3);

    return GetRayCollisionQuad(ray, p0, p1, p2, p3);
}

RayCollision RayIntersectsSkirting(const Ray& ray, const Wall& wall)
{
    const std::array<Vector3, 4> quad = wall.Skirting_quad();

    const Vector3 p0 = quad.at(0);
    const Vector3 p1 = quad.at(1);
    const Vector3 p2 = quad.at(2);
    const Vector3 p3 = quad.at(3);

    return GetRayCollisionQuad(ray, p0, p1, p2, p3);
}

Room::Room() noexcept
{
    Generate_box_room(4.0f, 5.0f, 2.5f);
}

void Room::Generate_box_room(float width, float length, float height) noexcept
{
    corners.clear();

    const Vector3 p0 = { -half_of(width), 0, -half_of(length) };
    const Vector3 p1 = { half_of(width), 0, -half_of(length) };
    const Vector3 p2 = { half_of(width), 0, half_of(length) };
    const Vector3 p3 = { -half_of(width), 0, half_of(length) };

    const Vector3 p4 = { -half_of(width), height , -half_of(length) };
    const Vector3 p5 = { half_of(width),  height , -half_of(length) };
    const Vector3 p6 = { half_of(width),  height , half_of(length) };
    const Vector3 p7 = { -half_of(width), height , half_of(length) };

    corners = { p0, p1, p2, p3, p4, p5, p6, p7 };


    walls.clear();
    walls.reserve(4);
    walls.emplace_back(Wall({ 0, 1, 5, 4 }, &corners));
    walls.emplace_back(Wall({ 1, 2, 6, 5 }, &corners));
    walls.emplace_back(Wall({ 2, 3, 7, 6 }, &corners));
    walls.emplace_back(Wall({ 3, 0, 4, 7 }, &corners));

    walls.push_back(Wall({ 3, 2, 1, 0 }, &corners));
    floor_index = walls.size() - 1;
    walls.at(floor_index).skirt_board.Set_height(0.0f); //TODO: demeter

    walls.push_back(Wall({ 4, 5, 6, 7 }, &corners));
    cieling_index = walls.size() - 1;
    walls.at(cieling_index).skirt_board.Set_height(0.0f); //TODO: demeter

    walls.at(0).Try_Add_Door();

}

float Room::Total_wall_paint_area() const noexcept
{
    float area = 0.0f;
    for (const Wall& wall : walls)
    {
        area += wall.Wall_paint_area();
    }
    return area;
}

float Room::Selected_wall_area() const
{
    float area = 0.0f;
    for (const Wall& wall : selected_walls)
        area += wall.Wall_paint_area();
    return area;
}

float Room::Liters_of(const Paint* target) const
{
    float total = 0.0f;
    for (const auto& wall : walls)
    {
        total += wall.Liters_of(target);
    }
    return total;
}

void Room::Mirror_resize(const Wall& dragged_wall, const Vector3& move_delta)
{
    if (!dragged_wall.room_vertices) return;

    const Vector3 direction = dragged_wall.Normal();

    for (Vector3& corner : corners)
    {
        const Vector3 to_corner = Vector3Subtract(corner, position);
        const float side = Vector3DotProduct(to_corner, direction);

        if (side > 0.0f)
            corner = Vector3Add(corner, move_delta);
        else if (side < 0.0f)
            corner = Vector3Subtract(corner, move_delta);
    }
}

void Room::Draw_walls() const
{
    if (walls.size() < 3) return;


    for (int i = 0; i < walls.size(); i++)
    {
        if (i == floor_index)
        {
            walls.at(i).Draw_filled(DARKGRAY);

            walls.at(i).Draw_Area(TextAnchor3D::Center);
        }
        else if (i == cieling_index)
        {
            walls.at(i).Draw_filled(WHITE);
        }
        else
        {
            walls.at(i).Draw();

        }
    }
}

Paint::Paint()
{
    color = { 250, 150, 150, 255 };
    coats = 2;
    m2_per_liter = 10.0f;
}

Wall::Handle::Handle() :
    hovered{ false },
    selected{ false },
    wall{ nullptr },
    last_hit{ 0.0f, 0.0f, 0.0f }
{}