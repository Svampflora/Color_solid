#pragma once
#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning(disable:ALL_CODE_ANALYSIS_WARNINGS)
#include "raylib.h"
#include <raymath.h>
#pragma warning(pop)

#include "Utilities.h"
#include <vector>
#include <string>
#include <regex>

using RGB = Color;

struct Lab_Color
{
    float L, a, b;
};

static inline float pivot_rgb(float n) noexcept {
    return (n > 0.04045f) ? powf((n + 0.055f) / 1.055f, 2.4f) : (n / 12.92f);
}

static inline float pivot_lab(float t) noexcept {
    return (t > 0.008856f) ? powf(t, 1.0f / 3.0f) : (7.787f * t + 16.0f / 116.0f);
}

static inline constexpr float inv_pivot_lab(float t) noexcept {
    const float t3 = t * t * t;
    return (t3 > 0.008856f) ? t3 : ((t - 16.0f / 116.0f) / 7.787f);
}

Lab_Color RGB_to_Lab(RGB c) noexcept;
RGB Lab_to_RGB(Lab_Color lab) noexcept;
Lab_Color RGB_to_OKLab(RGB c) noexcept;
RGB OKLab_to_RGB(Lab_Color lab) noexcept;
// uppskattning
RGB NCS_To_RGB(const std::string& ncsCode);


RGB RGB_lerp(RGB a, RGB b, float t) noexcept;
RGB Lab_lerp(RGB a, RGB b, float t) noexcept;
RGB OKlab_lerp(RGB a, RGB b, float t) noexcept;
RGB HSV_lerp(RGB a, RGB b, float t) noexcept;


float deltaE76(Lab_Color c1, Lab_Color c2);

inline void drawTriangleGradient(Vector2 v1, Vector2 v2, Vector2 v3, Color c1, Color c2, unsigned int resolution)
{
    for (unsigned int i = 0; i < resolution; ++i)
    {
        const float t0 = static_cast<float>(i) / resolution;
        const float t1 = static_cast<float>(i + 1) / resolution;
         
        const Vector2 v2_outer = Vector2Lerp(v2, v1, t0);
        const Vector2 v3_outer = Vector2Lerp(v3, v1, t0);
         
        const Vector2 v2_inner = Vector2Lerp(v2, v1, t1);
        const Vector2 v3_inner = Vector2Lerp(v3, v1, t1);
         
        const Color c = OKlab_lerp(c1, c2, t0);

        DrawTriangle(v2_outer, v3_outer, v2_inner, c);
        DrawTriangle(v3_outer, v3_inner, v2_inner, c);
    }
}



Lab_Color mix_colors(const std::vector<Lab_Color>& colors, const std::vector<float>& weights);

constexpr RGB NCS_GREEN{ 0, 158, 107, 255 };
constexpr RGB NCS_RED {196, 3, 51, 255};
constexpr RGB NCS_YELLOW {255, 212, 0, 255};
constexpr RGB NCS_BLUE {0, 135, 189, 255};

struct NCS_Color 
{
    int blackness;         
    int chromaticness;     
    std::string hueCode;   
    RGB rgb = WHITE;

    NCS_Color() = default;

    NCS_Color(const std::string& ncsStr)
    {
        parse_from_string(ncsStr);
    }

    NCS_Color(int _blackness, int _chromaticness, const std::string& _hueCode)
    {
        blackness = std::min(_blackness, 99);
        chromaticness = std::min(_chromaticness, 99);
        hueCode = (_chromaticness == 0) ? "N" : _hueCode;
        rgb = NCS_To_RGB(to_string());
    }

    std::string to_string() const 
    {


        char buf[32];
        std::snprintf(buf, sizeof(buf), "S %02d%02d-%s", blackness, chromaticness, hueCode.c_str());
        return std::string(buf);
    }

    void draw(Vector2 position, Vector2 size) const noexcept
    {
        DrawRectangleV(position, size, rgb);
        DrawTextF(this->to_string().c_str(), position.x, position.y + size.y, narrow_cast<int>(0.5f * size.y), WHITE);
    }

private:

    void parse_from_string(const std::string& ncsStr)
    {
        std::regex re("S\\s?(\\d{2})(\\d{2})-([A-Z0-9]+)");
        std::smatch match;

        if (std::regex_match(ncsStr, match, re))
        {
            blackness = std::stoi(match[1]);
            chromaticness = std::stoi(match[2]);
            hueCode = (chromaticness == 0) ? "N" : match[3].str();
            rgb = NCS_To_RGB(ncsStr);

            blackness = std::min(blackness, 99);
            chromaticness = std::min(chromaticness, 99);

        }
        else
        {
            throw std::invalid_argument("Ogiltig NCS-kod: " + ncsStr);
        }
    }
};

class NCSTriangle
{
    std::string hueCode;             
    unsigned int resolution = 10;    
    std::vector<NCS_Color> NCScolors;

public:
    NCSTriangle(std::string _hueCode, unsigned int _resolution)
        : hueCode(_hueCode), resolution(_resolution)
    {
        generateColors();
    }

    void draw(Vector2 position, Vector2 size)
    {
        const float cellW = size.x / resolution;
        const float cellH = size.y / resolution;


        for (unsigned int i = 0; i <= resolution; ++i)
        {
            for (unsigned int j = 0; j <= resolution - i; ++j)
            {
                const int blackness = i * (100 / resolution);
                const int chromaticness = j * (100 / resolution);

                const NCS_Color ncs = { blackness, chromaticness, hueCode};
                const Vector2 pos{ position.x + j * cellW,
                                    position.y + i * cellH + (0.5f * j * cellH)};
                 
                const float square_size = 0.5f * (size.y / resolution);

                ncs.draw(pos, { square_size, square_size });
            }
        }
    }

private:
    void generateColors()
    {
        NCScolors.clear();
        for (unsigned int i = 0; i <= resolution; ++i)
        {
            for (unsigned int j = 0; j <= resolution - i; ++j)
            {
                const int blackness = i * (100 / resolution);
                const int chromaticness = j * (100 / resolution);
                NCScolors.push_back({ blackness, chromaticness, hueCode });
            }
        }
    }
};

class ColorWheel
{
public:
    struct Node
    {
        float angle;
        RGB color;
    };

private:
    std::vector<Node> wheel;

public:
    ColorWheel(const std::vector<Node>& nodes)
    {
        wheel = nodes;
        std::sort(wheel.begin(), wheel.end(), [](const Node& a, const Node& b) {
            return a.angle < b.angle;
            });

        // Lägg till en kopia av första färgen på slutet (för interpolering över 2π)
        if (!wheel.empty()) 
        {
            Node wrap = wheel.front();
            wrap.angle += 2 * PI;
            wheel.push_back(wrap);
        }
    }

    void draw(Vector2 position, float radius, unsigned int resolution) const 
    {
        for (unsigned int i = 0; i < resolution; ++i)
        {
            const float angle = (2 * PI * i) / resolution;
            const RGB color = get_color(angle);
            const Vector2 dir = { cosf(angle), sinf(angle) };

            const float theta = 2 * PI / resolution;
            const float squareSize = 2.0f * radius * sinf(theta / 2.0f);

            const Vector2 center = {
            position.x + dir.x * (radius + squareSize / 2.0f),
            position.y + dir.y * (radius + squareSize / 2.0f)
            };

            const Rectangle rect = {
                center.x,
                center.y,
                squareSize,
                squareSize
            };

            DrawRectanglePro(
                rect,
                {  squareSize / 2 , squareSize / 2 },
                angle * RAD2DEG,
                color
            );
        }
    }

    RGB get_color(float radians) const noexcept
    {
        radians = fmod(radians, 2 * PI);
        if (radians < 0) radians += 2 * PI;

        for (size_t i = 0; i < wheel.size() - 1; ++i)
        {
            const Node& a = wheel[i];
            const Node& b = wheel[i + 1];

            if (radians >= a.angle && radians <= b.angle)
            {
                const float t = (radians - a.angle) / (b.angle - a.angle);
                return OKlab_lerp(a.color, b.color, t);
            }
        }

        return BLACK;
    }
private:
};

class ColorBicone3D
{
public:
    Vector3 position;
    Vector3 rotation;
    float radius;
    float height;
    unsigned int hue_resolution;
    unsigned int tint_resolution;
    const ColorWheel wheel;

    Model model;
    bool initialized = false;

    ColorBicone3D(Vector3 _position, float _radius, float _height, unsigned int _hue_resolution, unsigned int _tint_resolution, const ColorWheel& _wheel)
        : position(_position), radius(_radius), height(_height), hue_resolution(_hue_resolution), tint_resolution(_tint_resolution), wheel(_wheel)
    {
        buildModel();
    }

    ~ColorBicone3D()
    {
        if (initialized) UnloadModel(model);
    }

    void draw() const
    {
        if (!initialized) return;

        const Matrix rotationMatrix = MatrixRotateXYZ({ DEG2RAD * rotation.x, DEG2RAD * rotation.y, DEG2RAD * rotation.z });

        // Modellens transform
        DrawModelEx(model, position, { 0, 1, 0 }, rotation.y, { 1, 1, 1 }, WHITE);
        //DrawModelWiresEx(model, position, { 0, 1, 0 }, rotation.y, { 1, 1, 1 }, BLACK);
    }

private:

    void buildModel()
    {

        std::vector<Vector3> vertices;
        std::vector<Color> colors;

        const int segments = hue_resolution;
        const float angleStep = 2 * PI / segments;

        const float halfHeight = height * 0.5f;

        // ÖVRE HALVA: från vit → kulör (färghjul)
        for (int i = 0; i < segments; ++i)
        {
            float angle0 = i * angleStep;
            float angle1 = (i + 1) * angleStep;

            Color color0 = wheel.get_color(angle0);

            for (unsigned int r = 0; r < tint_resolution; ++r)
            {
                float t0 = (float)r / tint_resolution;
                float t1 = (float)(r + 1) / tint_resolution;

                // Vertikala nivåer
                float y0 = Lerp(0.0f, halfHeight, t0);
                float y1 = Lerp(0.0f, halfHeight, t1);

                // Radier minskar mot toppen
                float radius0 = radius * (1.0f - t0);
                float radius1 = radius * (1.0f - t1);

                Vector3 v0 = { cosf(angle0) * radius0, y0, sinf(angle0) * radius0 };
                Vector3 v1 = { cosf(angle1) * radius0, y0, sinf(angle1) * radius0 };
                Vector3 v2 = { cosf(angle0) * radius1, y1, sinf(angle0) * radius1 };
                Vector3 v3 = { cosf(angle1) * radius1, y1, sinf(angle1) * radius1 };

                Color c = OKlab_lerp( color0, WHITE, t0);

                // Två trianglar per rektangel-segment
                vertices.push_back(v0); colors.push_back(c);
                vertices.push_back(v2); colors.push_back(c);
                vertices.push_back(v3); colors.push_back(c);

                vertices.push_back(v0); colors.push_back(c);
                vertices.push_back(v3); colors.push_back(c);
                vertices.push_back(v1); colors.push_back(c);
            }
        }


        // UNDRE HALVA: från kulör (färghjul) → svart
        for (int i = 0; i < segments; ++i)
        {
            float angle0 = i * angleStep;
            float angle1 = (i + 1) * angleStep;

            Color color0 = wheel.get_color(angle0);

            for (unsigned int r = 0; r < tint_resolution; ++r)
            {
                float t0 = (float)r / tint_resolution;
                float t1 = (float)(r + 1) / tint_resolution;

                float y0 = Lerp(0.0f, -halfHeight, t0);
                float y1 = Lerp(0.0f, -halfHeight, t1);

                float radius0 = radius * (1.0f - t0);
                float radius1 = radius * (1.0f - t1);

                Vector3 v0 = { cosf(angle0) * radius0, y0, sinf(angle0) * radius0 };
                Vector3 v1 = { cosf(angle1) * radius0, y0, sinf(angle1) * radius0 };
                Vector3 v2 = { cosf(angle0) * radius1, y1, sinf(angle0) * radius1 };
                Vector3 v3 = { cosf(angle1) * radius1, y1, sinf(angle1) * radius1 };

                Color c = OKlab_lerp(color0, BLACK, t1);

                vertices.push_back(v0); colors.push_back(c);
                vertices.push_back(v3); colors.push_back(c);
                vertices.push_back(v2); colors.push_back(c);

                vertices.push_back(v0); colors.push_back(c);
                vertices.push_back(v1); colors.push_back(c);
                vertices.push_back(v3); colors.push_back(c);
            }
        }

        // Skapa modell
        Mesh mesh = {};
        mesh.triangleCount = static_cast<int>(vertices.size() / 3);
        mesh.vertexCount = static_cast<int>(vertices.size());

        mesh.vertices = (float*)MemAlloc(sizeof(float) * 3 * mesh.vertexCount);
        mesh.colors = (unsigned char*)MemAlloc(sizeof(unsigned char) * 4 * mesh.vertexCount);

        for (int i = 0; i < mesh.vertexCount; ++i)
        {
            mesh.vertices[i * 3 + 0] = vertices[i].x;
            mesh.vertices[i * 3 + 1] = vertices[i].y;
            mesh.vertices[i * 3 + 2] = vertices[i].z;

            mesh.colors[i * 4 + 0] = colors[i].r;
            mesh.colors[i * 4 + 1] = colors[i].g;
            mesh.colors[i * 4 + 2] = colors[i].b;
            mesh.colors[i * 4 + 3] = colors[i].a;
        }

        UploadMesh(&mesh, false);
        model = LoadModelFromMesh(mesh);
        initialized = true;
    }
};




