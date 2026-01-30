#pragma once

#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning(disable:ALL_CODE_ANALYSIS_WARNINGS)
#include "raylib.h"
#pragma warning(pop)

#include <vector>
#include <string>
#include <regex>
#include <map>
#include <numeric>

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
Lab_Color RGB_to_OKLab(RGB c) noexcept;
RGB Lab_to_RGB(Lab_Color lab) noexcept;
RGB OKLab_to_RGB(Lab_Color lab) noexcept;
RGB HSV_to_RGB(float h, float s, float v) noexcept;
RGB HSL_to_RGB(float h, float s, float l) noexcept;


// uppskattning
RGB NCS_To_RGB(const std::string& ncsCode);


RGB RGB_lerp(RGB a, RGB b, float t) noexcept;
RGB Lab_lerp(RGB a, RGB b, float t) noexcept;
RGB OKlab_lerp(RGB a, RGB b, float t) noexcept;
RGB HSV_lerp(RGB a, RGB b, float t) noexcept;

void DrawTriangleGradient(Vector2 v1, Vector2 v2, Vector2 v3, Color c1, Color c2, unsigned int resolution);

float deltaE76(Lab_Color c1, Lab_Color c2);


class Color_wheel
{
public:
    struct Spoke
    {
        float angle;
        RGB color;

    };

private:
    std::vector<Spoke> spokes;

public:
    Color_wheel();
    Color_wheel(const std::vector<Spoke>& nodes);
    void draw(Vector2 position, float radius, unsigned int resolution) const noexcept;
    RGB get_color(float radians) const noexcept;

private:
};


Lab_Color mix_colors(const std::vector<Lab_Color>& colors, const std::vector<float>& weights);

constexpr RGB NCS_GREEN{ 0, 158, 107, 255 };
constexpr RGB NCS_RED {196, 3, 51, 255};
constexpr RGB NCS_YELLOW {255, 212, 0, 255};
constexpr RGB NCS_BLUE {0, 135, 189, 255};

//struct NCS_Color 
//{
//    int blackness;         
//    int chromaticness;     
//    std::string hueCode;   
//    RGB rgb = { 255, 255, 255, 255 };
//
//    NCS_Color(const std::string& ncsStr);
//    NCS_Color(int _blackness, int _chromaticness, const std::string& _hueCode);
//    Color_Plus to_NCSPlus() const;
//    std::string to_string() const;
//    void draw(Vector2 position, Vector2 size) const;
//
//
//private:
//
//    void parse_from_string(const std::string& ncsStr);
//
//    float hueCode_to_radians(const std::string& _hueCode) const;
//
//    std::map<std::string, float> hue_Angles() const;
//};
//
//class NCSTriangle
//{
//    std::string hueCode;             
//    unsigned int resolution = 10;    
//    std::vector<NCS_Color> NCScolors;
//
//public:
//    NCSTriangle(std::string _hueCode, unsigned int _resolution)
//        : hueCode(_hueCode), resolution(_resolution)
//    {
//        generateColors();
//    }
//
//
//    void draw(Vector2 position, Vector2 size);
//
//private:
//    void generateColors();
//
//};

constexpr float COLOR_NODE_RADIUS = 0.15f; //settings
struct Color_node
{
    Vector3 position;
    RGB color;
};

class Color_solid
{
public:
    const Color_wheel wheel;
    float radius;
    float height;
    unsigned int angle_steps;    // hue resolution
    unsigned int radial_steps;   // saturation resolution
    std::vector<Color_node> color_nodes;


    Color_solid(float _radius, float _height, unsigned int _hue_resolution, unsigned int _sat_resolution, const Color_wheel& _wheel): 
        wheel(_wheel),
        radius(_radius), 
        height(_height), 
        angle_steps(_hue_resolution), 
        radial_steps(_sat_resolution),
        color_nodes{}
    {
        Make_nodes();
    }

    void Draw() const;
    Color_node node(size_t index) const { return color_nodes.at(index); }


private:
    void Make_nodes();

};




