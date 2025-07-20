#pragma once

#include "State.h"
#include "ColorUtils.h"
#include "Endscreen.h"
#include <format>
#include <array>
#include <optional>

//------------ text utils --------------------
#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning(disable:ALL_CODE_ANALYSIS_WARNINGS)
#include "rlgl.h"
#include <raymath.h>

struct RayHit
{
    bool hit;
    Vector3 point;
};
RayHit RayIntersectPlane(Ray ray, Vector3 planeNormal, float planeDistance)
{
    RayHit result{ false, Vector3Zero() };

    const float denom = Vector3DotProduct(planeNormal, ray.direction);
    if (fabs(denom) > 1e-6f) 
    {
        const float t = -(Vector3DotProduct(planeNormal, ray.position) + planeDistance) / denom;
        if (t >= 0)
        {
            result.hit = true;
            result.point = Vector3Add(ray.position, Vector3Scale(ray.direction, t));
        }
    }
    return result;
}

enum class TextAnchor3D
{
    Center,
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight,
    TopCenter,
    BottomCenter,
    MiddleLeft,
    MiddleRight
};

static inline void DrawTextCodepoint3D(Font font, int codepoint, Vector3 position, float fontSize, bool backface, Color tint) 
{
    // Character index position in sprite font
    // NOTE: In case a codepoint is not available in the font, index returned points to '?'
    const int index = GetGlyphIndex(font, codepoint);
    const float scale = fontSize / (float)font.baseSize;

    // Character destination rectangle on screen
    // NOTE: We consider charsPadding on drawing
    position.x += (float)(font.glyphs[index].offsetX - font.glyphPadding) * scale;
    position.z += (float)(font.glyphs[index].offsetY - font.glyphPadding) * scale;

    // Character source rectangle from font texture atlas
    // NOTE: We consider chars padding when drawing, it could be required for outline/glow shader effects
    const Rectangle srcRec = { font.recs[index].x - (float)font.glyphPadding, font.recs[index].y - (float)font.glyphPadding,
                         font.recs[index].width + 2.0f * font.glyphPadding, font.recs[index].height + 2.0f * font.glyphPadding };

    const float width = (float)(font.recs[index].width + 2.0f * font.glyphPadding) * scale;
    const float height = (float)(font.recs[index].height + 2.0f * font.glyphPadding) * scale;

    if (font.texture.id > 0)
    {
        const float x = 0.0f;
        const float y = 0.0f;
        const float z = 0.0f;

        // normalized texture coordinates of the glyph inside the font texture (0.0f -> 1.0f)
        const float tx = srcRec.x / font.texture.width;
        const float ty = srcRec.y / font.texture.height;
        const float tw = (srcRec.x + srcRec.width) / font.texture.width;
        const float th = (srcRec.y + srcRec.height) / font.texture.height;


        rlCheckRenderBatchLimit(4 + 4 * backface);
        rlSetTexture(font.texture.id);

        rlPushMatrix();
        rlTranslatef(position.x, position.y, position.z);

        rlBegin(RL_QUADS);
        rlColor4ub(tint.r, tint.g, tint.b, tint.a);

        // Front Face
        rlNormal3f(0.0f, 1.0f, 0.0f);                                   // Normal Pointing Up
        rlTexCoord2f(tx, ty); rlVertex3f(x, y, z);                      // Top Left Of The Texture and Quad
        rlTexCoord2f(tx, th); rlVertex3f(x, y, z + height);              // Bottom Left Of The Texture and Quad
        rlTexCoord2f(tw, th); rlVertex3f(x + width, y, z + height);     // Bottom Right Of The Texture and Quad
        rlTexCoord2f(tw, ty); rlVertex3f(x + width, y, z);              // Top Right Of The Texture and Quad

        if (backface)
        {
            // Back Face
            rlNormal3f(0.0f, -1.0f, 0.0f);                              // Normal Pointing Down
            rlTexCoord2f(tx, ty); rlVertex3f(x, y, z);                   // Top Right Of The Texture and Quad
            rlTexCoord2f(tw, ty); rlVertex3f(x + width, y, z);          // Top Left Of The Texture and Quad
            rlTexCoord2f(tw, th); rlVertex3f(x + width, y, z + height); // Bottom Left Of The Texture and Quad
            rlTexCoord2f(tx, th); rlVertex3f(x, y, z + height);         // Bottom Right Of The Texture and Quad
        }
        rlEnd();
        rlPopMatrix();

        rlSetTexture(0);
    }
}
static inline void DrawText3D(Font font, const char* text, Vector3 position, float fontSize, float fontSpacing, float lineSpacing, bool backface, Color tint)
{
    int length = TextLength(text);          // Total length in bytes of the text, scanned by codepoints in loop

    float textOffsetY = 0.0f;               // Offset between lines (on line break '\n')
    float textOffsetX = 0.0f;               // Offset X to next character to draw

    float scale = fontSize / (float)font.baseSize;

    for (int i = 0; i < length;)
    {
        // Get next codepoint from byte string and glyph index in font
        int codepointByteCount = 0;
        int codepoint = GetCodepoint(&text[i], &codepointByteCount);
        int index = GetGlyphIndex(font, codepoint);

        // NOTE: Normally we exit the decoding sequence as soon as a bad byte is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol moving one byte
        if (codepoint == 0x3f) codepointByteCount = 1;

        if (codepoint == '\n')
        {
            // NOTE: Fixed line spacing of 1.5 line-height
            // TODO: Support custom line spacing defined by user
            textOffsetY += fontSize + lineSpacing;
            textOffsetX = 0.0f;
        }
        else
        {
            if ((codepoint != ' ') && (codepoint != '\t'))
            {
                DrawTextCodepoint3D(font, codepoint, Vector3{ position.x + textOffsetX, position.y, position.z + textOffsetY }, fontSize, backface, tint);
            }

            if (font.glyphs[index].advanceX == 0) textOffsetX += (float)font.recs[index].width * scale + fontSpacing;
            else textOffsetX += (float)font.glyphs[index].advanceX * scale + fontSpacing;
        }

        i += codepointByteCount;   // Move text bytes counter to next codepoint
    }
}
Vector3 GetAnchoredTextPosition3D(Font font, const char* text, Vector3 center, float fontSize, TextAnchor3D anchor)
{
    const int length = TextLength(text);
    float scale = fontSize / (float)font.baseSize;
    float textWidth = 0.0f;

    for (int i = 0; i < length;)
    {
        int codepointByteCount = 0;
        int codepoint = GetCodepoint(&text[i], &codepointByteCount);
        int index = GetGlyphIndex(font, codepoint);
        textWidth += (font.glyphs[index].advanceX > 0 ? font.glyphs[index].advanceX : font.recs[index].width) * scale;
        i += codepointByteCount;
    }

    float textHeight = fontSize;

    Vector3 offset = { 0 };

    switch (anchor)
    {
    case TextAnchor3D::Center:
        offset = { -textWidth * 0.5f, 0, -textHeight * 0.5f };
        break;
    case TextAnchor3D::TopLeft:
        offset = { 0, 0, 0 };
        break;
    case TextAnchor3D::TopRight:
        offset = { -textWidth, 0, 0 };
        break;
    case TextAnchor3D::BottomLeft:
        offset = { 0, 0, -textHeight };
        break;
    case TextAnchor3D::BottomRight:
        offset = { -textWidth, 0, -textHeight };
        break;
    case TextAnchor3D::TopCenter:
        offset = { -textWidth * 0.5f, 0, 0 };
        break;
    case TextAnchor3D::BottomCenter:
        offset = { -textWidth * 0.5f, 0, -textHeight };
        break;
    case TextAnchor3D::MiddleLeft:
        offset = { 0, 0, -textHeight * 0.5f };
        break;
    case TextAnchor3D::MiddleRight:
        offset = { -textWidth, 0, -textHeight * 0.5f };
        break;
    }

    return Vector3Add(center, offset);
}


static inline void DrawQuad(std::array<Vector3, 4> corners, Color color)
{
    DrawTriangle3D(corners[0], corners[1], corners[2], color);
    DrawTriangle3D(corners[2], corners[3], corners[0], color);
}
static inline void BuildTangentBasis(Vector3 normal, Vector3& right, Vector3& up)
{
    if (fabsf(normal.y) < 0.999f)
        right = Vector3Normalize(Vector3CrossProduct(normal, { 0, 1, 0 }));
    else
        right = Vector3Normalize(Vector3CrossProduct(normal, { 1, 0, 0 }));

    up = Vector3Normalize(Vector3CrossProduct(right, normal));
}
static inline void DrawRectangleLinesEx3D(Vector3 center, Vector2 size, Vector3 normal, float rotation, Color color)
{
    Vector3 right, up;
    BuildTangentBasis(normal, right, up);

    float cosAngle = cosf(rotation);
    float sinAngle = sinf(rotation);

    Vector3 rotatedRight = Vector3Scale(right, cosAngle);
    rotatedRight = Vector3Add(rotatedRight, Vector3Scale(up, sinAngle));

    Vector3 rotatedUp = Vector3Scale(up, cosAngle);
    rotatedUp = Vector3Subtract(rotatedUp, Vector3Scale(right, sinAngle));

    float halfW = size.x / 2.0f;
    float halfH = size.y / 2.0f;

    Vector3 topLeft = Vector3Add(center, Vector3Add(Vector3Scale(rotatedUp, halfH), Vector3Scale(rotatedRight, -halfW)));
    Vector3 topRight = Vector3Add(center, Vector3Add(Vector3Scale(rotatedUp, halfH), Vector3Scale(rotatedRight, +halfW)));
    Vector3 bottomRight = Vector3Add(center, Vector3Add(Vector3Scale(rotatedUp, -halfH), Vector3Scale(rotatedRight, +halfW)));
    Vector3 bottomLeft = Vector3Add(center, Vector3Add(Vector3Scale(rotatedUp, -halfH), Vector3Scale(rotatedRight, -halfW)));

    DrawLine3D(topLeft, topRight, color);
    DrawLine3D(topRight, bottomRight, color);
    DrawLine3D(bottomRight, bottomLeft, color);
    DrawLine3D(bottomLeft, topLeft, color);
}
static inline void DrawPolygonLinesEx3D(const std::vector<Vector3>& points, Color color)
{
    if (points.size() < 2) return;

    for (size_t i = 0; i < points.size(); ++i)
    {
        const Vector3& a = points[i];
        const Vector3& b = points[(i + 1) % points.size()];

        DrawLine3D(a, b, color);
    }
}
static inline void DrawTriangleFan3D(const std::vector<Vector3>& points, Color color)
{
    size_t vertex_minimum = 3;
    if (points.size() < vertex_minimum) return;

    rlCheckRenderBatchLimit(3 * (narrow_cast<int>(points.size()) - 2));
    rlBegin(RL_TRIANGLES);
    rlColor4ub(color.r, color.g, color.b, color.a);

    const Vector3 center = points[0];
    for (size_t i = 1; i < points.size() - 1; ++i)
    {
        rlVertex3f(center.x, center.y, center.z);
        rlVertex3f(points[i].x, points[i].y, points[i].z);
        rlVertex3f(points[i + 1].x, points[i + 1].y, points[i + 1].z);
    }

    rlEnd();
}

#pragma warning(pop)



inline const char* FormatMeasurement(float meters)
{
    return (meters >= 1.0f)
        ? TextFormat("%.1f M", meters)
        : TextFormat("%.0f CM", meters * 100.0f);
}



struct Attribute
{
    enum class Type {Door, Window};

    float width = 1.0f, height = 2.0f;
    Type type = Type::Door;


    float Area() const noexcept
    {
        return width * height;
    }
};

struct Wall
{
    struct Handle
    {
        bool hovered{ false };
        bool selected{ false };
        Wall* wall = nullptr;
    };

    std::vector<size_t> corner_indices;
    const std::vector< Vector3>* room_corners = nullptr;
    std::vector<Attribute> openings{};

    Wall(const std::vector<size_t>& indices, const std::vector<Vector3>* corners_ptr) :
        corner_indices(indices), 
        room_corners(corners_ptr)
    {}

    const Vector3& Corner(size_t i) const 
    {
        if (!room_corners) {
            throw std::runtime_error("room_corners pointer is null");
        }
        if (i >= corner_indices.size()) {
            throw std::out_of_range("Corner index out of bounds in Wall");
        }
        size_t corner_i = corner_indices.at(i);
        if (corner_i >= room_corners->size()) {
            throw std::out_of_range("Wall::corner_indices contains invalid index");
        }

        return (*room_corners).at(corner_indices.at(i));
    }

    //bool Try_add_opening(const Attribute& opening)
    //{
    //    const float spacing = 0.5f;
    //    float totalWidth = 0.0f;
    //    for (const auto& o : openings)
    //    {
    //       totalWidth += o.width + spacing;
    //    }
    //    totalWidth += opening.width;
    //    if (totalWidth + spacing * openings.size() <= Length())
    //    {
    //        openings.push_back(opening);
    //        return true;
    //    }
    //    return false;
    //}
    float Length() const
    {
        if (room_corners->size() < 2) return 0.0f;

        Vector3 lowest1 = Corner(0);
        Vector3 lowest2 = Corner(1);

        if (lowest2.y < lowest1.y)
            std::swap(lowest1, lowest2);

        for (size_t i = 0; i < corner_indices.size(); i++)
        {
           
            if (Corner(i).y < lowest1.y)
            {
                lowest2 = lowest1;
                lowest1 = Corner(i);
            }
            else if (Corner(i).y < lowest2.y)
            {
                lowest2 = Corner(i);
            }
        }

        constexpr float tolerance = 0.001f;
        if (std::abs(lowest1.y - lowest2.y) > tolerance)
            return 0.0f;

        return Vector2Distance({ lowest1.x, lowest1.z }, { lowest2.x, lowest2.z });
    }
    float Height() const
    {
        if (room_corners->size() < 2) return 0.0f;

        float min_y = Corner(0).y;
        float max_y = Corner(0).y;

        for (const auto& c : *room_corners)
        {
            if (c.y < min_y) min_y = c.y;
            if (c.y > max_y) max_y = c.y;
        }

        return max_y - min_y;
    }
    float Area() const
    {
        if (room_corners->size() < 3) return 0.0f;

        std::vector<Vector3> points = Corners();

        const Vector3 u = Vector3Subtract(points[1], points[0]);         
        const Vector3 u_dir = Vector3Normalize(u);
        const Vector3 v_dir = Vector3Normalize(Vector3CrossProduct(Normal(), u_dir));

        std::vector<Vector2> projected;
        const Vector3 origin = points[0];

        for (const Vector3& p : points) {
            const Vector3 rel = Vector3Subtract(p, origin);
            const float x = Vector3DotProduct(rel, u_dir);
            const float y = Vector3DotProduct(rel, v_dir);
            projected.push_back({ x, y });
        }

        float area = 0.0f;
        const size_t n = projected.size();
        for (size_t i = 0; i < n; ++i) {
            const Vector2& a = projected[i];
            const Vector2& b = projected[(i + 1) % n];
            area += (a.x * b.y - b.x * a.y);
        }

        return std::abs(area * 0.5f);
    }
    Vector3 Center() const
    {
        if (room_corners->empty()) return Vector3Zero();

        Vector3 sum = Vector3Zero();
        for (size_t i = 0; i < corner_indices.size(); ++i)
        {
            sum = Vector3Add(sum, Corner(i)); //TODO: fix error
        }

        return Vector3Scale(sum, 1.0f / static_cast<float>(corner_indices.size()));
    }
    Vector3 Normal() const
    {
        if (room_corners->size() < 3) return Vector3Zero();

        const Vector3 u = Vector3Subtract(Corner(1), Corner(0));
        const Vector3 v = Vector3Subtract(Corner(2), Corner(0));
        return Vector3Normalize(Vector3CrossProduct(u, v));
    }

    std::vector<Vector3> Corners() const
    {
        if (!room_corners || room_corners->size() < 3)
            throw std::runtime_error("Wall::Corners() called with null or too few corners.");

        std::vector<Vector3> points;
        points.reserve(corner_indices.size());

        for (size_t i = 0; i < corner_indices.size(); ++i)
        {
            points.emplace_back(Corner(i));
        }
        return points;
    }
    std::array<Vector3, 4> Quad() const
    {
        if (room_corners->size() < 4) throw;


        const Vector3 p0 = Corner(0);
        const Vector3 p1 = Corner(1);
        const Vector3 p2 = Corner(2);
        const Vector3 p3 = Corner(3);

        return{ p0, p1, p2, p3 };
    }
    std::array<Vector3, 3> Triangle() const
    {
        if (room_corners->size() < 3) throw;


        const Vector3 p0 = Corner(0);
        const Vector3 p1 = Corner(1);
        const Vector3 p2 = Corner(2);

        return{ p0, p1, p2 };
    }

    void Draw( const Color color) const
    {
        const Vector3 up = { 0.0f, 1.0f, 0.0f };

        DrawPolygonLinesEx3D(Corners(), color);
    }
    void Draw_filled(const Color color) const
    {
        DrawQuad(Quad(), color);
    }
};

static inline RayCollision RayIntersectsWall(const Ray& ray, const Wall& wall)
{
    const std::array<Vector3, 4> quad = wall.Quad();

    const Vector3 p0 = quad.at(0);
    const Vector3 p1 = quad.at(1);
    const Vector3 p2 = quad.at(2);
    const Vector3 p3 = quad.at(3);

    return GetRayCollisionQuad(ray, p0, p1, p2, p3);
}

struct Room
{
	///float width{ 4.0f }, length{ 5.0f }, height{ 2.5f};
    Vector3 position{ 0.0f, 0.0f, 0.0f };

    std::vector<Vector3> corners{};
    std::vector<Wall> walls{};
    std::vector<Wall> selected_walls{};
    size_t floor_index;

    Room()
    {
        Generate_box_room(4.0f, 5.0f, 2.5f);
    }

    void Generate_box_room(float width, float length, float height) noexcept
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

        walls.push_back(Wall({ 0, 1, 2, 3 }, &corners ));
        floor_index = walls.size() - 1;
    }
	float Total_wall_area() const noexcept
	{
        float area = 0.0f;
        for (const Wall& wall : walls)
        {
            area += wall.Area();
        }
        return area;
	}
    float Selected_wall_area() const noexcept
    {
        float area = 0.0f;
        for (const Wall& wall : selected_walls)
            area += wall.Area();
        return area;
    }

    //void MoveWall(Wall& wall, Vector3 newCenter)
    //{

    //}
    
    //void Mirror_resize(const Vector3& handlePoint, const Vector3& cursorWorldPos)
    //{

    //}
    
    //bool Add_door(Rectangle dimensions, ROOM_SURFACE surface)
    //{
    //    Attribute door{ dimensions.x, dimensions.y, Attribute::Type::Door };
    //    const float spacing = 0.5f;
    //}

    void Draw_floor(Color color) const 
    {
        if (walls.size() < 3) return;

        walls.at(floor_index).Draw_filled(color);
    }
    void Draw_walls(Color color) const
    {
        for (const Wall& wall : walls)
        {
            wall.Draw(color);
        }
    }

};

class Calculator
{
public:
	static float Liters_of_color(Room room, float liters_per_meter, unsigned int coats = 2) noexcept
	{
		return (coats * room.Total_wall_area()) / liters_per_meter;
	}
};

class Simulation : public State
{
	Camera3D camera = { 0 };
    Room room{};
    Vector2 camera_angle = { 0.0f , 0.0f };
    float camera_distance = 10.0f;
    unsigned int coats = 2;
    Wall::Handle handle{};

    float min_size = 1.0f;
    float max_size = 10.0f;


public:

	Simulation() noexcept
	{
		camera.position = { 0.0f, 10.0f, 10.0f };
		camera.target = Vector3{ 0.0f, 0.0f, 0.0f };
		camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
		camera.fovy = 55.0f;
		camera.projection = CAMERA_CUSTOM;

        room.Generate_box_room( 4.0f, 5.0f, 2.5f);

	};

	std::unique_ptr<State> Update() override
	{
		if (IsKeyReleased(KEY_Q))
		{
			return std::make_unique<End_screen>();
		}

        if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON))
        {
            const Vector2 mouse_delta = GetMouseDelta();
            camera_angle.x -= mouse_delta.x * 0.01f;
            camera_angle.y += mouse_delta.y * 0.01f;
        }

        camera_distance += GetMouseWheelMove() * -0.5f;
        camera_distance = Clamp(camera_distance, 2.0f, 50.0f);
        camera.position.x = sinf(camera_angle.x) * camera_distance;
        camera.position.z = cosf(camera_angle.y) * camera_distance;

        const Vector3 target = room.position;

        camera.position = 
        {
            target.x + camera_distance * cosf(camera_angle.y) * sinf(camera_angle.x),
            target.y + camera_distance * sinf(camera_angle.y),
            target.z + camera_distance * cosf(camera_angle.y) * cosf(camera_angle.x)
        };
        camera.target = target;

        Edit();

		return nullptr;
	};



    void Edit()
    {
        const Vector2 mouse = GetMousePosition();
        constexpr float radius = 10.0f;

        for (auto& wall : room.walls)
        {
            const Vector2 screen_position = GetWorldToScreen(wall.Center(), camera);
            if (CheckCollisionPointCircle(mouse, screen_position, radius))
            {
                handle.wall = &wall;
                handle.hovered = true;

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    handle.selected = true;
                }


                DrawCircleV(screen_position, radius, RED);
            }
            else
            {
                handle = Wall::Handle{};
                DrawCircleV(screen_position, 4, GRAY);
            }
        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        {
            handle = Wall::Handle{};
        }

       if (handle.hovered && IsMouseButtonDown(MOUSE_LEFT_BUTTON))
        {

            const Ray ray = GetMouseRay(GetMousePosition(), camera);
            const RayHit hit = RayIntersectPlane(ray, { 0, 1, 0 }, -room.position.y);
            const RayHit ceiling_hit = RayIntersectPlane(ray, { 1, 0, 0 }, -room.position.x);


            Vector3 hit_point = Vector3Zero();
            Vector3 ceiling_hit_point = Vector3Zero();


            if (hit.hit)
            {
                hit_point = hit.point;

            }

            if (ceiling_hit.hit)
            {
                ceiling_hit_point = ceiling_hit.point;

            }

            //room.Mirror_resize(handle.wall->Center(), hit.point);
        }

    }
        //if (active_handle != ROOM_SURFACE::None && IsKeyPressed(KEY_D))
        //{
        //    room.Add_door({ 1.0f, 2.0f }, active_handle);
        //}

	void Render() const override
	{
        BeginMode3D(camera);

        room.Draw_floor(DARKGRAY);
        room.Draw_walls(WHITE);



        //DrawText3D(GetFontDefault(), FormatMeasurement(room.width), { room_position.x - half_of(room.width), room_position.y - half_of(room.height), room_position.y + half_of(room.length) }, 0.4f,0.1f,1.0f, true, WHITE);


        //rlPushMatrix();
        //rlRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        //DrawText3D(GetFontDefault(), FormatMeasurement(room.length), { room_position.x - half_of(room.length), room_position.y - half_of(room.height), room_position.y + half_of(room.width) }, 0.4f, 0.1f, 1.0f, true, WHITE);
        //DrawText3D(GetFontDefault(), TextFormat("%.1f M2", room.Floor_area()), { room_position.x , room_position.y - 0.49f * room.height, room_position.y }, 0.4f, 0.1f, 1.0f, true, WHITE);

        //rlPopMatrix();


        //rlPushMatrix();
        //rlRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        //DrawText3D(GetFontDefault(), FormatMeasurement(room.height), { room_position.x + half_of(room.width), room_position.y - half_of(room.length), room_position.y - half_of(room.height) }, 0.4f, 0.1f, 1.0f, true, WHITE);
        //rlPopMatrix();


        EndMode3D();

        //const float liters = Calculator::Liters_of_color(room, 8.0f, coats);
        //DrawText(TextFormat("Beräknad färgåtgång: %.1f L", liters), 20, 40, 50, RAYWHITE);
        //DrawText(TextFormat("Golvyta: %.1f M2", room.Floor_area()), 20, 80, 50, RAYWHITE);
        // DrawText(TextFormat("Strykningar: %i st", coats), 20, 120, 50, RAYWHITE);


	};

};