#pragma once

#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning(disable:ALL_CODE_ANALYSIS_WARNINGS)
#include "raylib.h"
#pragma warning(pop)

#include <vector>
#include <array>

struct RayHit {
    bool hit;
    Vector3 point;
};

RayHit RayIntersectPlane(Ray ray, Vector3 planeNormal, float planeDistance);
bool IntersectLineWithHorizontalPlane(Vector3 a, Vector3 b, float planeY, Vector3& outPoint);
Vector3 RotationMatrixToEuler(Matrix m);
void MatrixToEulerZYX(const Matrix& mat, float& yaw, float& pitch, float& roll);

enum class TextAnchor3D {
    Center, TopLeft, TopRight, BottomLeft, BottomRight,
    TopCenter, BottomCenter, MiddleLeft, MiddleRight
};

// === Text drawing ===
void DrawTextCodepoint3D(Font font, int codepoint, Vector3 position, float fontSize, bool backface, Color tint);
void DrawText3D(Font font, const char* text, Vector3 position, float fontSize, float fontSpacing, float lineSpacing, bool backface, Color tint);
Vector3 GetAnchoredTextOffset3D(Font font, const char* text, float fontSize, TextAnchor3D anchor);
void DrawAnchoredText3D(Font font, const char* text, Vector3 position, float fontSize, float fontSpacing, bool backface, Color tint, TextAnchor3D anchor, const Matrix& rotation);

// === Geometry helpers ===
void DrawQuad(std::array<Vector3, 4> corners, Color color);
void BuildTangentBasis(Vector3 normal, Vector3& right, Vector3& up);
void DrawRectangleLinesEx3D(Vector3 center, Vector2 size, Vector3 normal, float rotation, Color color);
void DrawPolygonLinesEx3D(const std::vector<Vector3>& points, Color color);
void DrawQuadLinesEx3D(const std::array<Vector3, 4>& points, Color color);
void DrawTriangleFan3D(const std::vector<Vector3>& points, Color color);
void DrawCubeWires3D(Vector3 position, float width, float height, float depth, Color tint, const Matrix& rotation);


Vector3 PolygonNormal(const std::vector<Vector3>& vertices);
float PolygonArea(const std::vector<Vector3>& vertices);
float TriangleArea(const Vector3& a, const Vector3& b, const Vector3& c);
float QuadArea(const std::array<Vector3, 4>& v);

const char* FormatMeasurement(float meters) noexcept;

namespace debugging_tools
{
    void DrawVertexOrder(const std::array<Vector3, 4>& w, const Vector3 normal);

    //DrawLine3D(Center(), Vector3Add(Center(), Vector3Scale(right, 1.0f)), RED);
//DrawLine3D(Center(), Vector3Add(Center(), Vector3Scale(up, 1.0f)), BLUE);
//DrawLine3D(Center(), Vector3Add(Center(), Vector3Scale(forward, 1.0f)), GREEN);

}