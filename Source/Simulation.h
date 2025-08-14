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
Vector3 RotationMatrixToEuler(Matrix m)
{
    Vector3 euler;

    // Check for gimbal lock
    if (fabsf(m.m2) < 0.999f) {
        euler.y = asinf(-m.m2); // pitch (Y)
        euler.x = atan2f(m.m6, m.m10); // yaw (X)
        euler.z = atan2f(m.m1, m.m0);  // roll (Z)
    }
    else {
        // Gimbal lock fallback
        euler.y = asinf(-m.m2); // pitch (Y)
        euler.x = atan2f(-m.m9, m.m5); // yaw (X)
        euler.z = 0.0f;
    }

    // Convert to degrees
    euler.x = RAD2DEG * euler.x;
    euler.y = RAD2DEG * euler.y;
    euler.z = RAD2DEG * euler.z;

    return euler;
}
void MatrixToEulerZYX(const Matrix& mat, float& yaw, float& pitch, float& roll)
{
    if (fabsf(mat.m6) < 0.999f)  // No gimbal lock
    {
        pitch = asinf(-mat.m6);                    // -sin(pitch)
        roll = atan2f(mat.m4, mat.m5);             // atan2(xz / yy)
        yaw = atan2f(mat.m2, mat.m10);             // atan2(zy / zz)
    }
    else
    {
        // Gimbal lock
        pitch = asinf(-mat.m6);
        roll = 0;
        yaw = atan2f(-mat.m8, mat.m0);
    }

    // Convert to degrees for rlRotatef
    yaw = RAD2DEG * yaw;
    pitch = RAD2DEG * pitch;
    roll = RAD2DEG * roll;
}

//static Vector3 ClosestPointOnLineToRay(const Ray& ray, const Vector3& linePoint, const Vector3& lineDir)
//{
//    Vector3 u = Vector3Normalize(ray.direction); // ray direction
//    Vector3 v = Vector3Normalize(lineDir);       // line direction (the wall normal)
//    Vector3 w0 = Vector3Subtract(ray.position, linePoint);
//
//    float a = Vector3DotProduct(u, u); // ~1
//    float b = Vector3DotProduct(u, v);
//    float c = Vector3DotProduct(v, v); // ~1
//    float d = Vector3DotProduct(u, w0);
//    float e = Vector3DotProduct(v, w0);
//
//    float denom = a * c - b * b;
//    float tc; // parameter on the line (linePoint + tc * v)
//
//    if (fabsf(denom) < 1e-6f) {
//        // Almost parallel: choose tc so the line point is the projection of ray origin onto the line:
//        tc = e / c;
//    }
//    else {
//        // general case
//        // sc = (b*e - c*d) / denom;  // parameter on the ray (unused here)
//        tc = (a * e - b * d) / denom;
//    }
//
//    return Vector3Add(linePoint, Vector3Scale(v, tc));
//}

#include <cmath>

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
static inline void DrawText3D(Font font, const char* text, Vector3 position, float fontSize, float fontSpacing, float lineSpacing, bool backface, Color tint/*, const Matrix& transform*/)
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
Vector3 GetAnchoredTextOffset3D(Font font, const char* text, float fontSize, TextAnchor3D anchor)
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

    return offset;
}
void DrawAnchoredText3D(Font font, const char* text, Vector3 position,
    float fontSize, float fontSpacing, bool backface, Color tint,
    TextAnchor3D anchor, const Matrix& rotation)
{
    const Vector3 default_text_forward = { 0.0f, 1.0f, 0.0f };
    const Vector3 default_text_up = { 0.0f, 0.0f, -1.0f };
    const Vector3 default_text_right = Vector3Normalize(Vector3CrossProduct(default_text_up, default_text_forward));

    const Matrix default_rotation =
    {
        default_text_right.x, default_text_up.x, default_text_forward.x, 0,
        default_text_right.y, default_text_up.y, default_text_forward.y, 0,
        default_text_right.z, default_text_up.z, default_text_forward.z, 0,
        0, 0, 0, 1
    };

    float yaw, pitch, roll;
    MatrixToEulerZYX(rotation, yaw, pitch, roll);

    rlPushMatrix();
    rlTranslatef(position.x, position.y, position.z);

    if (!Vector3Equals({ rotation.m8, rotation.m9,rotation.m10 }, { 0.0f,1.0f,0.0f }))
    {
        roll += 90.0f;     // TODO: quick fix
    }
    rlRotatef(roll, 1, 0, 0);   // X 
    rlRotatef(pitch, 0, 1, 0);  // Y
    rlRotatef(yaw, 0, 0, 1);    // Z 


    Vector3 offset = GetAnchoredTextOffset3D(font, text, fontSize, anchor);
    DrawText3D(font, text, offset, fontSize, fontSpacing, fontSize, backface, tint);

    //DrawLine3D(offset, Vector3Add(offset, Vector3Scale(default_text_right, 0.5f)), RED);
    //DrawLine3D(offset, Vector3Add(offset, Vector3Scale(default_text_up, 0.5f)), BLUE);
    //DrawLine3D(offset, Vector3Add(offset, Vector3Scale(default_text_forward, 0.5f)), GREEN);

    rlPopMatrix();
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
        Vector3 last_hit{ 0.0f, 0.0f, 0.0f }; 
        
        Vector3 start_center{ 0.0f, 0.0f, 0.0f };
        float start_t{ 0.0f };
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
        const size_t corner_i = corner_indices.at(i);
        if (corner_i >= room_corners->size()) {
            throw std::out_of_range("Wall::corner_indices contains invalid index");
        }

        return (*room_corners).at(corner_indices.at(i));
    }
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

        for (const Vector3& p : points) 
        {
            const Vector3 rel = Vector3Subtract(p, origin);
            const float x = Vector3DotProduct(rel, u_dir);
            const float y = Vector3DotProduct(rel, v_dir);
            projected.push_back({ x, y });
        }

        float area = 0.0f;
        const size_t n = projected.size();
        for (size_t i = 0; i < n; ++i) 
        {
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
            sum = Vector3Add(sum, Corner(i));
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
        DrawPolygonLinesEx3D(Corners(), color);

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
            TextFormat("%.1f M2", Area()),
            world_pos,
            0.4f, 0.1f,
            false,
            WHITE,
            TextAnchor3D::Center,
            rotation
        );
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

        walls.push_back(Wall({ 3, 2, 1, 0 }, &corners ));
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
    

    void Mirror_resize(Wall& dragged_wall, const Vector3& move_delta)
    {
        if (!dragged_wall.room_corners) return;

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

        // 1. Hover detection
        Wall* hovered_wall = nullptr;
        float closest_distance_sq = radius * radius;

        for (auto& wall : room.walls)
        {
            const Vector2 screen_position = GetWorldToScreen(wall.Center(), camera);
            const float dist_sq = Vector2DistanceSqr(mouse, screen_position);

            if (dist_sq < closest_distance_sq)
            {
                closest_distance_sq = dist_sq;
                hovered_wall = &wall;
            }

            DrawCircleV(screen_position, hovered_wall == &wall ? radius : 4.0f,
                hovered_wall == &wall ? WHITE : GRAY);
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && hovered_wall)
        {
            handle.wall = hovered_wall;
            handle.selected = true;
            handle.last_hit = hovered_wall->Center();
        }

        if (handle.selected && handle.wall)
        {
            const Vector3 wall_normal = handle.wall->Normal();
            const Vector3 world_up = { 0.0f, 1.0f, 0.0f };
            const Vector3 sideways = Vector3Normalize(Vector3CrossProduct(world_up, wall_normal));
            const Vector3 perp_plane_normal = sideways;
            const Ray ray = GetMouseRay(mouse, camera);
            const float plane_d = Vector3DotProduct(perp_plane_normal, handle.wall->Center());
            const RayHit hit = RayIntersectPlane(ray, perp_plane_normal, plane_d);

            if (hit.hit)
            {
                const Vector3 center = handle.wall->Center();
                const Vector3 diff = Vector3Subtract(hit.point, center);
                const float t = Vector3DotProduct(diff, wall_normal); // distance along axis
                const Vector3 line_position = Vector3Add(center, Vector3Scale(wall_normal, t));
                const Vector3 move_delta = Vector3Subtract(line_position, handle.last_hit);
                handle.last_hit = line_position;

                room.Mirror_resize(*handle.wall, Vector3Negate(move_delta));
            }
        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        {
            handle = Wall::Handle{};
        }
    }

    //void Edit()
    //{
    //    const Vector2 mouse = GetMousePosition();
    //    constexpr float radius = 10.0f;

    //    // Hover detection
    //    Wall* hovered_wall = nullptr;
    //    float closest_dist_sq = radius * radius;
    //    for (auto& wall : room.walls)
    //    {
    //        const Vector2 screen_pos = GetWorldToScreen(wall.Center(), camera);
    //        const float dist_sq = Vector2DistanceSqr(mouse, screen_pos);
    //        if (dist_sq < closest_dist_sq) 
    //        {
    //            closest_dist_sq = dist_sq;
    //            hovered_wall = &wall;
    //        }
    //        DrawCircleV(screen_pos, hovered_wall == &wall ? radius : 4.0f,
    //            hovered_wall == &wall ? WHITE : GRAY);
    //    }
    //    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) 
    //    {
    //        handle = Wall::Handle{};
    //    }
    //    if (hovered_wall == nullptr)
    //    {
    //        return;
    //    }

    //    const Vector3 move_dir = hovered_wall->Normal(); 
    //    const Vector3 plane_normal = move_dir; 
    //    const Vector3 dir_norm = Vector3Normalize(plane_normal);
    //    const Ray ray = GetMouseRay(mouse, camera);
    //    const Vector3 right = Vector3Normalize(Vector3CrossProduct(world_up, forward));

    //    //const Ray direction_ray{ hovered_wall->Center(), plane_normal };
    //    const float plane_d = Vector3DotProduct(plane_normal, hovered_wall->Center());
    //    const RayHit hit = RayIntersectPlane(ray, plane_normal, plane_d);        

    //    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && hovered_wall)
    //    {

    //        if (hit.hit) 
    //        {
    //            handle.wall = hovered_wall;
    //            handle.last_hit = hit.point;
    //            handle.selected = true;

    //            const Vector3 to_hit = Vector3Subtract(hit.point, hovered_wall->Center());
    //            handle.start_t = Vector3DotProduct(to_hit, dir_norm); // store scalar along ray

    //            // Store the wall center position for reference
    //            handle.start_center = hovered_wall->Center();
    //        }
    //    }

    //    constexpr float min_dist = -2.0f; // how far you can move in the -normal direction
    //    constexpr float max_dist = 2.0f; // how far you can move in the +normal direction

    //    if (handle.selected && handle.wall)
    //    {
    //        const Vector3 to_hit = Vector3Subtract(hit.point, handle.start_center);
    //        const float current_t = Vector3DotProduct(to_hit, dir_norm);
    //        const float delta_t = current_t - handle.start_t;
    //        const float final_t = fmaxf(min_dist, fminf(max_dist, delta_t));
    //        const Vector3 line_position = Vector3Add(handle.start_center,
    //            Vector3Scale(dir_norm, final_t));

    //        const Vector3 move_delta = Vector3Subtract(line_position, handle.start_center);
    //        room.Mirror_resize(*handle.wall, move_delta);
    //    }
    //}


    //if (active_handle != ROOM_SURFACE::None && IsKeyPressed(KEY_D))
    //{
    //    room.Add_door({ 1.0f, 2.0f }, active_handle);
    //}

	void Render() const override
	{
        BeginMode3D(camera);

        room.Draw_floor(DARKGRAY);
        room.Draw_walls(WHITE);

        if (handle.wall)
        {
          handle.wall->Draw(GREEN);
          DrawSphere(handle.last_hit, 0.5f, GREEN);
          const Ray direction_ray{ handle.wall->Center(), handle.wall->Normal() };

          DrawRay(direction_ray, GREEN);

        }

        const Vector3 origo = { 0.0f, 5.0f, 0.0f };

        DrawLine3D(origo, Vector3Add(origo, Vector3Scale({ 1.0f, 0.0f, 0.0f }, 1.0f)), RED);
        DrawLine3D(origo, Vector3Add(origo, Vector3Scale({ 0.0f, 1.0f, 0.0f }, 1.0f)), BLUE);
        DrawLine3D(origo, Vector3Add(origo, Vector3Scale({ 0.0f, 0.0f, 1.0f }, 1.0f)), GREEN);

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