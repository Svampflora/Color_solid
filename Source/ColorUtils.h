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

static inline float inv_pivot_lab(float t) noexcept {
    float t3 = t * t * t;
    return (t3 > 0.008856f) ? t3 : ((t - 16.0f / 116.0f) / 7.787f);
}

Lab_Color RGB_to_Lab(Color c) noexcept;
Color Lab_to_RGB(Lab_Color lab) noexcept;

Color RGB_lerp(Color a, Color b, float t) noexcept;
Color Lab_lerp(Color a, Color b, float t) noexcept;
Color HSV_lerp(Color a, Color b, float t) noexcept;


float deltaE76(Lab_Color c1, Lab_Color c2);


// uppskattning
Color NCS_To_RGB(const std::string& ncsCode);

Lab_Color blendColors(const std::vector<Lab_Color>& colors, const std::vector<float>& weights);

constexpr Color NCS_GREEN{ 0, 158, 107, 255 };
constexpr Color NCS_RED {196, 3, 51, 255};
constexpr Color NCS_YELLOW {255, 212, 0, 255};
constexpr Color NCS_BLUE {0, 135, 189, 255};

struct NCS_Color 
{
    int blackness;          // Svarthetsvärde, 0–100
    int chromaticness;      // Kulörthet, 0–100
    std::string hueCode;    // T.ex. "Y20R"
    Color rgb = WHITE;

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
public:
    std::string hueCode;              // T.ex. "Y20R"
    unsigned int resolution = 10;     // Hur många steg (per axel)
    std::vector<NCS_Color> NCScolors;  // Förberedda färger

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
        float angle; // I radianer, t.ex. 0 till 2*PI
        Color color;
    };

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

    Color get_color(float radians) const noexcept
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
                return Lab_lerp(a.color, b.color, t);
            }
        }

        return BLACK; // Fallback
    }

    void draw(Vector2 position, float radius, unsigned int resolution) const 
    {
        for (unsigned int i = 0; i < resolution; ++i)
        {
            const float angle = (2 * PI * i) / resolution;
            const Color color = get_color(angle);
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

private:
    std::vector<Node> wheel;




};





