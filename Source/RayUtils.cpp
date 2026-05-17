#include "RayUtils.h"

#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning(disable:ALL_CODE_ANALYSIS_WARNINGS)
#include "raymath.h"
#include "rlgl.h"
#pragma warning(pop)

#include "Utilities.h"
#include <cmath>

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

RayCollision RayIntersectsQuad(const Ray& ray, const std::array<Vector3, 4>& quad)
{

    const Vector3 p0 = quad.at(0);
    const Vector3 p1 = quad.at(1);
    const Vector3 p2 = quad.at(2);
    const Vector3 p3 = quad.at(3);

    return GetRayCollisionQuad(ray, p0, p1, p2, p3);
}

bool IntersectLineWithHorizontalPlane(Vector3 a, Vector3 b, float planeY, Vector3& outPoint)
{
    // If both points are on the same side -> no intersection
    if ((a.y < planeY && b.y < planeY) || (a.y > planeY && b.y > planeY)) return false;

    // Linear interpolation along edge
    const float t = (planeY - a.y) / (b.y - a.y);
    outPoint = Vector3Lerp(a, b, t);
    return true;
}

Vector3 RotationMatrixToEuler(Matrix m) noexcept
{
    Vector3 euler{};

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

void MatrixToEulerZYX(const Matrix& mat, float& yaw, float& pitch, float& roll) noexcept
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

std::array<Vector3, 4> Quad_strip(const std::array<Vector3, 4>& quad, float normalized_left, float normalized_right)
{
    normalized_left = Clamp(normalized_left, 0.0f, 1.0f);
    normalized_right = Clamp(normalized_right, 0.0f, 1.0f);

    if (normalized_right < normalized_left)
        std::swap(normalized_left, normalized_right);


    const Vector3 br = quad[0];
    const Vector3 bl = quad[1];
    const Vector3 tl = quad[2];
    const Vector3 tr = quad[3];

    const Vector3 bottom_left_pos =
        Vector3Lerp(bl, br, normalized_left);

    const Vector3 bottom_right_pos =
        Vector3Lerp(bl, br, normalized_right);

    const Vector3 top_left_pos =
        Vector3Lerp(tl, tr, normalized_left);

    const Vector3 top_right_pos =
        Vector3Lerp(tl, tr, normalized_right);

    return { bottom_right_pos,
             bottom_left_pos,
             top_left_pos,
             top_right_pos };
}

void DrawTextCodepoint3D(Font font, int codepoint, Vector3 position, float fontSize, bool backface, Color tint)
{
    // Character index position in sprite font
    // NOTE: In case a codepoint is not available in the font, index returned points to '?'
    const int index = GetGlyphIndex(font, codepoint);
    const float scale = fontSize / static_cast<float>(font.baseSize);

    // Character destination rectangle on screen
    // NOTE: We consider charsPadding on drawing
    position.x += static_cast<float>(font.glyphs[index].offsetX - font.glyphPadding) * scale;
    position.z += static_cast<float>(font.glyphs[index].offsetY - font.glyphPadding) * scale;

    // Character source rectangle from font texture atlas
    // NOTE: We consider chars padding when drawing, it could be required for outline/glow shader effects
    const Rectangle srcRec = { font.recs[index].x - static_cast<float>(font.glyphPadding), font.recs[index].y - static_cast<float>(font.glyphPadding),
                         font.recs[index].width + 2.0f * font.glyphPadding, font.recs[index].height + 2.0f * font.glyphPadding };

    const float width =  static_cast<float>(font.recs[index].width + 2.0f * font.glyphPadding) * scale;
    const float height = static_cast<float>(font.recs[index].height + 2.0f * font.glyphPadding) * scale;

    if (font.texture.id > 0)
    {
        constexpr float x = 0.0f;
        constexpr float y = 0.0f;
        constexpr float z = 0.0f;

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

void DrawText3D(Font font, const char* text, Vector3 position, float fontSize, float fontSpacing, float lineSpacing, bool backface, Color tint)
{
    if(!text)
    {
        return;
    }
    const int length = TextLength(text);          // Total length in bytes of the text, scanned by codepoints in loop

    float textOffsetY = 0.0f;               // Offset between lines (on line break '\n')
    float textOffsetX = 0.0f;               // Offset X to next character to draw

    const float scale = fontSize / static_cast<float>(font.baseSize);

    for (int i = 0; i < length;)
    {
        // Get next codepoint from byte string and glyph index in font
        int codepointByteCount = 0;
        const int codepoint = GetCodepoint(&text[i], &codepointByteCount);
        const int index = GetGlyphIndex(font, codepoint);

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

            if (font.glyphs[index].advanceX == 0) textOffsetX += static_cast<float>(font.recs[index].width) * scale + fontSpacing;
            else textOffsetX += static_cast<float>(font.glyphs[index].advanceX) * scale + fontSpacing;
        }

        i += codepointByteCount;   // Move text bytes counter to next codepoint
    }
}

Vector3 GetAnchoredTextOffset3D(Font font, const char* text, float fontSize, TextAnchor3D anchor)
{
    const int length = TextLength(text);
    const float scale = fontSize / static_cast<float>(font.baseSize);
    float textWidth = 0.0f;

    for (int i = 0; i < length;)
    {
        int codepointByteCount = 0;
        const int codepoint = GetCodepoint(&text[i], &codepointByteCount);
        const int index = GetGlyphIndex(font, codepoint);
        textWidth += (font.glyphs[index].advanceX > 0 ? font.glyphs[index].advanceX : font.recs[index].width) * scale;
        i += codepointByteCount;
    }

    const float textHeight = fontSize;

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

void DrawAnchoredText3D(Font font, const char* text, Vector3 position, float fontSize, float fontSpacing, bool backface, Color tint, TextAnchor3D anchor, const Matrix& rotation)
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


    const Vector3 offset = GetAnchoredTextOffset3D(font, text, fontSize, anchor);
    DrawText3D(font, text, offset, fontSize, fontSpacing, fontSize, backface, tint);

    //DrawLine3D(offset, Vector3Add(offset, Vector3Scale(default_text_right, 0.5f)), RED);
    //DrawLine3D(offset, Vector3Add(offset, Vector3Scale(default_text_up, 0.5f)), BLUE);
    //DrawLine3D(offset, Vector3Add(offset, Vector3Scale(default_text_forward, 0.5f)), GREEN);

    rlPopMatrix();
}

void DrawCubeWires3D(Vector3 position, float width, float height, float depth, Color tint, const Matrix& rotation)
{
    const Vector3 default_forward = { 0.0f, 1.0f, 0.0f };
    const Vector3 default_up = { 0.0f, 0.0f, -1.0f };
    const Vector3 default_right = Vector3Normalize(Vector3CrossProduct(default_up, default_forward));

    const Matrix default_rotation =
    {
        default_right.x, default_up.x, default_forward.x, 0,
        default_right.y, default_up.y, default_forward.y, 0,
        default_right.z, default_up.z, default_forward.z, 0,
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


    DrawCubeWires(position, width, height, depth, tint);
    //DrawLine3D(offset, Vector3Add(offset, Vector3Scale(default_text_right, 0.5f)), RED);
    //DrawLine3D(offset, Vector3Add(offset, Vector3Scale(default_text_up, 0.5f)), BLUE);
    //DrawLine3D(offset, Vector3Add(offset, Vector3Scale(default_text_forward, 0.5f)), GREEN);

    rlPopMatrix();
}

void DrawQuad(std::array<Vector3, 4> corners, Color color) noexcept
{
    DrawTriangle3D(corners[0], corners[1], corners[2], color);
    DrawTriangle3D(corners[2], corners[3], corners[0], color);
}

void BuildTangentBasis(Vector3 normal, Vector3& right, Vector3& up)
{
    if (fabsf(normal.y) < 0.999f)
        right = Vector3Normalize(Vector3CrossProduct(normal, { 0, 1, 0 }));
    else
        right = Vector3Normalize(Vector3CrossProduct(normal, { 1, 0, 0 }));

    up = Vector3Normalize(Vector3CrossProduct(right, normal));
}

void DrawRectangleLinesEx3D(Vector3 center, Vector2 size, Vector3 normal, float rotation, Color color)
{
    Vector3 right, up;
    BuildTangentBasis(normal, right, up);

    const float cosAngle = cosf(rotation);
    const float sinAngle = sinf(rotation);

    Vector3 rotatedRight = Vector3Scale(right, cosAngle);
    rotatedRight = Vector3Add(rotatedRight, Vector3Scale(up, sinAngle));

    Vector3 rotatedUp = Vector3Scale(up, cosAngle);
    rotatedUp = Vector3Subtract(rotatedUp, Vector3Scale(right, sinAngle));

    const float halfW = size.x / 2.0f;
    const float halfH = size.y / 2.0f;
     
    const Vector3 topLeft = Vector3Add(center, Vector3Add(Vector3Scale(rotatedUp, halfH), Vector3Scale(rotatedRight, -halfW)));
    const Vector3 topRight = Vector3Add(center, Vector3Add(Vector3Scale(rotatedUp, halfH), Vector3Scale(rotatedRight, +halfW)));
    const Vector3 bottomRight = Vector3Add(center, Vector3Add(Vector3Scale(rotatedUp, -halfH), Vector3Scale(rotatedRight, +halfW)));
    const Vector3 bottomLeft = Vector3Add(center, Vector3Add(Vector3Scale(rotatedUp, -halfH), Vector3Scale(rotatedRight, -halfW)));

    DrawLine3D(topLeft, topRight, color);
    DrawLine3D(topRight, bottomRight, color);
    DrawLine3D(bottomRight, bottomLeft, color);
    DrawLine3D(bottomLeft, topLeft, color);
}

void DrawPolygonLinesEx3D(const std::vector<Vector3>& points, Color color) noexcept
{
    if (points.size() < 2) return;

    for (size_t i = 0; i < points.size(); ++i)
    {
        const Vector3& a = points[i];
        const Vector3& b = points[(i + 1) % points.size()];

        DrawLine3D(a, b, color);
    }
}

void DrawQuadLinesEx3D(const std::array<Vector3, 4>& points, Color color) noexcept
{
    if (points.size() < 2) return;

    for (size_t i = 0; i < points.size(); ++i)
    {
        const Vector3& a = points[i];
        const Vector3& b = points[(i + 1) % points.size()];

        DrawLine3D(a, b, color);
    }
}

void DrawTriangleFan3D(const std::vector<Vector3>& points, Color color)
{
    constexpr size_t vertex_minimum = 3;
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



Quaternion IntegrateRotation(Quaternion current, Vector3 angular_velocity, float dt)
{
    const float speed = Vector3Length(angular_velocity);

    if (speed < 0.00001f)
        return current;

    const Vector3 axis = Vector3Scale(angular_velocity, 1.0f / speed);
    const float angle = speed * dt;  // radians this frame
    const Quaternion dq = QuaternionFromAxisAngle(axis, angle);

    // Apply incremental rotation
    current = QuaternionMultiply(dq, current);

    return QuaternionNormalize(current);
}

Vector3 PolygonNormal(const std::vector<Vector3>& vertices)
{
    if (vertices.size() < 3) return Vector3Zero();

    const Vector3 u = Vector3Subtract(vertices[1], vertices[0]);
    const Vector3 v = Vector3Subtract(vertices[2], vertices[0]);
    return Vector3Normalize(Vector3CrossProduct(u, v));
}

float PolygonArea(const std::vector<Vector3>& vertices)
{
    if (vertices.size() < 3) return 0.0f;

    std::vector<Vector3> points = vertices;

    const Vector3 u = Vector3Subtract(points[1], points[0]);
    const Vector3 u_dir = Vector3Normalize(u);
    const Vector3 v_dir = Vector3Normalize(Vector3CrossProduct(PolygonNormal(vertices), u_dir));

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

float TriangleArea(const Vector3& a, const Vector3& b, const Vector3& c)
{
    const Vector3 ab = Vector3Subtract(b, a);
    const Vector3 ac = Vector3Subtract(c, a);
    const Vector3 cross = Vector3CrossProduct(ab, ac);
    return 0.5f * Vector3Length(cross);
}

float QuadArea(const std::array<Vector3, 4>& v)
{
    // Quad vertices assumed in order:
    // v0 -- v1
    // |     |
    // v3 -- v2

    // Split into two triangles: (v0, v1, v2) and (v0, v2, v3)
    const float A1 = TriangleArea(v[0], v[1], v[2]);
    const float A2 = TriangleArea(v[0], v[2], v[3]);

    return A1 + A2;
}

inline const char* FormatMeasurement(float meters) noexcept
{
    return (meters >= 1.0f)
        ? TextFormat("%.1f M", meters)
        : TextFormat("%.0f CM", meters * 100.0f);
}

void debugging_tools::DrawVertexOrder(const std::array<Vector3, 4>& w, const Vector3 normal)
{

    const Vector3 forward = Vector3Negate(normal);
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
    for (int i = 0; i < w.size(); i++)
    {
        DrawCircle3D(w.at(i), 0.1f, Vector3Zero(), 0, BLUE);
        DrawAnchoredText3D
        (
            GetFontDefault(),
            TextFormat("%i", i),
            w.at(i),
            0.2f, 0.1f,
            false,
            RED,
            TextAnchor3D::Center,
            rotation
        );
    }
    
}

Rectangle TextRect(Font font, const char* text, Vector2 position, float fontSize, float spacing)
{
    const Vector2 size = MeasureTextEx(font, text, fontSize, spacing);

    return {
        position.x,
        position.y,
        size.x,
        size.y
    };
}

Rectangle TextRect(Font font,  const std::string& text, Vector2 position, float fontSize, float spacing)
{
    return TextRect(font, text.c_str(), position, fontSize, spacing);
}