#include "Room.h"

#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning(disable:ALL_CODE_ANALYSIS_WARNINGS)
#include "raymath.h"
#include "rlgl.h"
#pragma warning(pop)

#include "RayUtils.h"
#include "Utilities.h"
#include <format>
#include <string>


bool Handle::Hovered(const Camera& camera) const noexcept
{
    if (!Active())
    {
        return false;
    }
    const Vector2 mouse = GetMousePosition();
    constexpr float radius = 10.0f;
    float range = radius * radius;
    const Vector2 screen_position = GetWorldToScreen(Position(), camera);
    const float distance = Vector2DistanceSqr(mouse, screen_position);
    if (distance > range)
    {
        return false;
    }
    return true;
}

// === Aperture ===

float Aperture::Area() const noexcept
{
    return width * height;
}

float Aperture::Height() const noexcept
{
    return height;
}

float Aperture::Width() const noexcept
{
    return width;
}

std::array<Vector3, 4> Aperture::Quad(const std::array<Vector3, 4>& w, const Vector3 wall_normal) const
{
    const Vector3 forward = wall_normal;
    const Vector3 world_up = { 0.0f, 1.0f, 0.0f };
    const Vector3 right = Vector3Normalize(Vector3CrossProduct(world_up, forward));
    const Vector3 up = Vector3Normalize(Vector3CrossProduct(forward, right));

    //const float wall_width = Vector3Distance(w.at(0), w.at(1));
    //const float wall_height = Vector3Distance(w.at(0), w.at(3));
    const Vector3 aperture_center = Center_position(w);
        //Vector3Add(w.at(0), Vector3Add(Vector3Scale(right, (center.x * wall_width)), Vector3Scale(up, (center.y * wall_height))));

    const Vector3 mid_right = Vector3Scale(right, half_of(width));
    const Vector3 mid_top = Vector3Scale(up, half_of(height));

    return
    {
        Vector3Subtract(aperture_center, Vector3Add(mid_right, mid_top)) ,
        Vector3Add(aperture_center, Vector3Subtract(mid_right, mid_top)),
        Vector3Add(aperture_center, Vector3Add(mid_right, mid_top)),
        Vector3Subtract(aperture_center, Vector3Subtract(mid_right, mid_top))
    };
}

Vector3 Aperture::Center_position(const std::array<Vector3, 4>& wall_quad) const
{
    const Vector3 right = Vector3Normalize(Vector3Subtract(wall_quad[1], wall_quad[0]));
    const Vector3 up = Vector3Normalize(Vector3Subtract(wall_quad[3], wall_quad[0]));
    const float wall_width = Vector3Distance(wall_quad[0], wall_quad[1]);
    const float wall_height = Vector3Distance(wall_quad[0], wall_quad[3]);

    return Vector3Add(wall_quad.at(0), Vector3Add(Vector3Scale(right, (center.x * wall_width)), Vector3Scale(up, (center.y * wall_height))));
}

void Aperture::Draw(const std::array<Vector3, 4>& wall_quad, const Vector3& wall_normal, const Color& color) const
{
    auto window_vertices = Quad(wall_quad, wall_normal);
    const Vector3 w_br = window_vertices[0], w_bl = window_vertices[1], w_tl = window_vertices[2], w_tr = window_vertices[3];

    const Vector3 away{ Vector3Scale(Vector3Negate(wall_normal), depth) };
    const Vector3 w_br_away = Vector3Add(w_br, away), w_bl_away = Vector3Add(w_bl, away), w_tl_away = Vector3Add(w_tl, away), w_tr_away = Vector3Add(w_tr, away);

    //left plane
    DrawQuadLinesEx3D({ w_bl, w_tl, w_tl_away, w_bl_away }, color);

    //bottom plane
    DrawQuadLinesEx3D({ w_bl, w_bl_away, w_br_away, w_br }, color);

    //top plane
    DrawQuadLinesEx3D({ w_tl_away, w_tl, w_tr, w_tr_away }, color);

    //right plane
    DrawQuadLinesEx3D({ w_br_away, w_tr_away, w_tr, w_br }, color);
}

// === DOOR ===

Entrance::Entrance(const float& _center, const float& wall_height) noexcept
{
    width = 1.0f;
    height=2.0f;
    architrave = 0.05f;
    center = { _center,  half_of(height) / wall_height };
}



std::array<Vector3, 4> Entrance::Frame_quad(const std::array<Vector3, 4>& w, const Vector3 wall_normal) const
{
    const Vector3 forward = wall_normal;
    constexpr Vector3 world_up = { 0.0f, 1.0f, 0.0f };
    const Vector3 right = Vector3Normalize(Vector3CrossProduct(world_up, forward));
    const Vector3 up = Vector3Normalize(Vector3CrossProduct(forward, right));

    const std::array<Vector3, 4>& door_quad = Quad(w, wall_normal);

    const Vector3 p0 = Vector3Add(door_quad.at(0), Vector3Scale(Vector3Negate(right), architrave));          // bottom left
    const Vector3 p1 = Vector3Add(door_quad.at(1), Vector3Scale(right, architrave));                         // bottom right
    const Vector3 p2 = Vector3Add(p1, Vector3Scale(up, Frame_height()));                                     // top right
    const Vector3 p3 = Vector3Add(p0, Vector3Scale(up, Frame_height()));                                     // top left 


    return{ p0, p1, p2, p3 };
}

Vector3 Entrance::Center_position(const std::array<Vector3, 4>& wall_quad) const
{
    const Vector3 right = Vector3Normalize(Vector3Subtract(wall_quad[1], wall_quad[0]));
    const Vector3 up = Vector3Normalize(Vector3Subtract(wall_quad[3], wall_quad[0]));
    const float wall_width = Vector3Distance(wall_quad[0], wall_quad[1]);
    //const float wall_height = Vector3Distance(wall_quad[0], wall_quad[3]);

    return Vector3Add(wall_quad.at(0), Vector3Add(Vector3Scale(right, (center.x * wall_width)), Vector3Scale(up, (half_of(height)))));
}

float Entrance::Frame_height() const noexcept
{
    return height + architrave;
}

float Entrance::Frame_width() const noexcept
{
    return width + architrave * 2;
}


void Entrance::Draw(const std::array<Vector3, 4>& wall_quad, const Vector3& wall_normal, const Color& color) const
{

    auto door_vertices = Quad(wall_quad, wall_normal);
    const Vector3 w_br = door_vertices.at(0), w_bl = door_vertices.at(1), w_tl = door_vertices.at(2), w_tr = door_vertices.at(3);

    DrawQuadLinesEx3D(door_vertices, color);
    DrawQuadLinesEx3D(Frame_quad(wall_quad, wall_normal), color);
}




// === SKIRTING ===

Color Skirting::Get_color() const 
{
    if (paint_layers.empty())
    {
        throw std::runtime_error("skirting has no color to provide");
    }
    return paint_layers.front()->color;
}

float Skirting::Area(const std::array<Vector3, 4>& wall_quad, const std::vector<Entrance>& entrances, const Vector3& wall_normal) const 
{
    float area = 0.0f;
    std::vector< std::array<Vector3, 4>> boards = Quads(wall_quad, entrances, wall_normal);

    for (const auto& board : boards)
    {
        area += QuadArea(board);
    }
    return area;
}

std::vector<std::array<Vector3, 4>>
Skirting::Quads(const std::array<Vector3, 4>& wall_quad,
    const std::vector<Entrance>& entrances,
    const Vector3 wall_normal) const
{
    std::vector<std::array<Vector3, 4>> boards;

    const Vector3 vertical = Vector3Normalize(Vector3Subtract(wall_quad.at(3), wall_quad.at(0)));
    const Vector3 height_vector = Vector3Scale(vertical, height);

    Vector3 bottom_left = wall_quad.at(0);

    for (const auto& entrance : entrances)
    {
        auto frame = entrance.Frame_quad(wall_quad, wall_normal);

        const Vector3 entrance_left = frame.at(0);

        boards.push_back({
            bottom_left,
            entrance_left,
            Vector3Add(entrance_left, height_vector),
            Vector3Add(bottom_left, height_vector)
            });

        bottom_left = frame.at(1);
    }

    const Vector3 final_right = wall_quad.at(1);

    boards.push_back({
        bottom_left,
        final_right,
        Vector3Add(final_right, height_vector),
        Vector3Add(bottom_left, height_vector)
        });

    return boards;
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

void Skirting::Draw(const std::array<Vector3, 4>& wall_quad, const std::vector<Entrance>& entrances, const Vector3& wall_normal, const Color& color) const
{
    std::vector< std::array<Vector3, 4>> boards = Quads(wall_quad, entrances, wall_normal);

    for (const auto& board : boards)
    {
        DrawQuad(board, color);
        debugging_tools::DrawVertexOrder(board, wall_normal);
    }
}

void Skirting::Draw_outline(const std::array<Vector3, 4>& wall_quad, const std::vector<Entrance>& entrances, const Vector3& wall_normal, const Color& color) const
{
    std::vector< std::array<Vector3, 4>> boards = Quads(wall_quad, entrances, wall_normal);

    for (const auto& board : boards)
    {
        DrawQuadLinesEx3D(board, color);
    }
}




// === WALL ===

Wall::Wall(const std::vector<size_t>& indices, const std::vector<Vector3>* corners_ptr) noexcept :
    vertex_indices(indices),
    room_vertices(corners_ptr)
{}

//Wall::Handle::Handle() :
//    hovered{ false },
//    selected{ false },
//    wall{ nullptr },
//    last_hit{ 0.0f, 0.0f, 0.0f }
//{}

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

void Wall::Add_paint(Paint& paint)
{
    paint_layers.push_back(&paint);
}

void Wall::Try_add_door() 
{

    const Entrance door(0.5f, Height());
    float total_door_width = door.Frame_width();
    for (const auto& _door : doors)
    {
        total_door_width += _door.Frame_width();
    }
    if (total_door_width >= Length()) //TODO: make function Availible_edge_space(); take mouse position into account
    {
        return;
    }

    //TODO: squeeze in door if possible
    doors.push_back(door);
}

void Wall::Try_add_aperture() noexcept
{
    windows.push_back(Aperture({0.5f, 0.5f}));
}

void Wall::Draw_area(const TextAnchor3D anchor) const
{
    const Vector3 forward = Normal();
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

void Wall::Draw_distance(const Vector3& a, const Vector3& b, const Color& color, const TextAnchor3D anchor) const
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
}

Vector3 Wall::Position(const Vector2& normalized_coordinate) const
{
    const std::array<Vector3, 4> quad = Quad();
    const Vector3 right = Vector3Normalize(Vector3Subtract(quad[1], quad[0]));
    const Vector3 up = Vector3Normalize(Vector3Subtract(quad[3], quad[0]));
    const float wall_width = Vector3Distance(quad[0], quad[1]);
    const float wall_height = Vector3Distance(quad[0], quad[3]);

    return Vector3Add(quad.at(0), Vector3Add(Vector3Scale(right, (normalized_coordinate.x * wall_width)), Vector3Scale(up, (normalized_coordinate.y * wall_height))));
}

void Wall::Draw_doors_outline(const Color color) const
{
    if (doors.empty())
    {
        return;
    }

    for (auto& door : doors)
    {
        door.Draw(Quad(), Normal(), color);
    }
}

void Wall::Draw_apertures_outline(const Color& color) const
{
    if (windows.empty())
    {
        return;
    }

    for (auto& window : windows)
    {
        window.Draw(Wall_paint_quad(), Normal(), color);
    }
}

//void Wall::Draw_skirting_outline(const Color color) const
//{
//
//
//    std::array<Vector3, 4> arr = Skirting_quad();
//    std::vector <Vector3> vec(arr.begin(), arr.end());
//    DrawPolygonLinesEx3D(vec, color);
//    debugging_tools::DrawVertexOrder(arr, Normal());
//
//}

void Wall::Draw_filled() const
{

    if (!paint_layers.empty())
    {
        Draw_filled(paint_layers.front()->color);
    }

}


void Wall::Draw_filled(const Color& color) const
{
    const auto wall_vertices = Wall_paint_quad();
    const Vector3 br = wall_vertices.at(0), bl = wall_vertices.at(1), tl = wall_vertices.at(2), tr = wall_vertices.at(3);

    if (windows.empty() && doors.empty())
    {
        DrawQuad(Wall_paint_quad(), color);
        return;
    }

    for (const Aperture& window : windows)
    {
        auto window_vertices = window.Quad(wall_vertices, Normal());
        const Vector3 w_br = window_vertices.at(0), w_bl = window_vertices.at(1), w_tl = window_vertices.at(2), w_tr = window_vertices.at(3);


        // Top 
        DrawQuad({ tl,
                   tr,
                   w_tr,
                   w_tl }, color);

        // Bottom 
        DrawQuad({ br,
                   bl,
                   w_bl,
                   w_br }, color);

        // Left
        DrawQuad({ tl,
                   w_tl,
                   w_bl,
                   bl }, color);

        // Right 
        DrawQuad({ w_tr,
                   tr,
                   br,
                   w_br}, color);


    }

    constexpr Vector3 world_up = { 0.0f, 1.0f, 0.0f };
    const Vector3 right = Vector3Normalize(Vector3CrossProduct(world_up, Normal()));
    const Vector3 up = Vector3Normalize(Vector3CrossProduct(Normal(), right));

    //br = Vector3Add(br, Vector3Scale(up, skirt_board.height));
    //bl = Vector3Add(bl, Vector3Scale(up, skirt_board.height));

    for (const auto& door : doors)
    {
        auto door_vertices = door.Frame_quad(Quad(), Normal());
        Vector3 d_br = door_vertices.at(0), d_bl = door_vertices.at(1), d_tl = door_vertices.at(2), d_tr = door_vertices.at(3);

        d_br = Vector3Add(d_br, Vector3Scale(up, skirt_board.height));
        d_bl = Vector3Add(d_bl, Vector3Scale(up, skirt_board.height));

        // Top 
        DrawQuad({ tl,
                   tr,
                   d_tr,
                   d_tl },color);

        // Left
        DrawQuad({ tl,
                   d_tl,
                   d_bl,
                   bl }, color);

        // Right 
        DrawQuad({ d_tr,
                   tr,
                   br,
                   d_br }, color);


    }
}

//void Wall::Draw_skirting_filled() const
//{
//    DrawQuad(Skirting_quad(), skirt_board.Get_color());
//
//}
//
//void Wall::Draw_skirting_filled(const Color& _color) const
//{
//    DrawQuad(Skirting_quad(), _color);
//
//}

void Wall::Draw() const
{
    const Color text_color = WHITE;

    if (!paint_layers.empty())
    {
        Draw_filled();
    }

    Draw_outline(WHITE);
    Draw_area(TextAnchor3D::Center);
    Draw_distance(Vertex(0), Vertex(1), text_color, TextAnchor3D::TopCenter);
    Draw_distance(Vertex(0), Vertex(3), text_color, TextAnchor3D::MiddleLeft);

    if (skirt_board.Is_painted())
    {
        skirt_board.Draw(Quad(), doors, Normal(), skirt_board.Get_color()); //Todo: make internal colorcheck instead of .get
    }
    else
    {
        skirt_board.Draw_outline(Quad(), doors, Normal(), WHITE);
    }

    if (!doors.empty())
    {
        Draw_doors_outline(WHITE);
    }

    if (!windows.empty())
    {
        Draw_apertures_outline(WHITE);
    }
}



RayCollision RayIntersectsWall(const Ray& ray, const Wall& wall) //TODO: find home
{
    const std::array<Vector3, 4> quad = wall.Quad();

    const Vector3 p0 = quad.at(0);
    const Vector3 p1 = quad.at(1);
    const Vector3 p2 = quad.at(2);
    const Vector3 p3 = quad.at(3);

    return GetRayCollisionQuad(ray, p0, p1, p2, p3);
}

RayCollision RayIntersectsSkirting(const Ray& ray, const Wall& wall) //TODO: copy
{
    const std::array<Vector3, 4> quad = wall.Skirting_quad();

    const Vector3 p0 = quad.at(0);
    const Vector3 p1 = quad.at(1);
    const Vector3 p2 = quad.at(2);
    const Vector3 p3 = quad.at(3);

    return GetRayCollisionQuad(ray, p0, p1, p2, p3);
}


// === ROOM ===

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

    walls.at(0).Try_add_door();
    walls.at(1).Try_add_aperture();


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

void Room::Mirror_resize(const Vector3& direction, const Vector3& move_delta)
{

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
            if (walls.at(i).paint_layers.empty())
            {
                walls.at(i).Draw_filled(DARKGRAY);
            }
            else
            {
                walls.at(i).Draw_filled();
            }
            walls.at(i).Draw_area(TextAnchor3D::Center);
        }
        else if (i == cieling_index)
        {
            if (walls.at(i).paint_layers.empty())
            {
                walls.at(i).Draw_filled(WHITE);
            }
            else
            {
                walls.at(i).Draw_filled();
            }
        }
        else
        {
            walls.at(i).Draw();

        }
    }
}
