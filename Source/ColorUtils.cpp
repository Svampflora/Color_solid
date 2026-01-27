#include "ColorUtils.h"

#pragma warning(push)
#pragma warning(disable:ALL_CODE_ANALYSIS_WARNINGS)
#include "raymath.h"
#pragma warning(pop)

#include "Utilities.h"
#include "RayUtils.h"
#include <cmath>
#include <map>

Lab_Color RGB_to_Lab(RGB color) noexcept
{
    const float r = pivot_rgb(color.r / 255.0f);
    const float g = pivot_rgb(color.g / 255.0f);
    const float b = pivot_rgb(color.b / 255.0f);
    
    // sRGB D65 → XYZ
    float x = (r * 0.4124f + g * 0.3576f + b * 0.1805f) / 0.95047f;
    float y = (r * 0.2126f + g * 0.7152f + b * 0.0722f) / 1.00000f;
    float z = (r * 0.0193f + g * 0.1192f + b * 0.9505f) / 1.08883f;

    x = pivot_lab(x);
    y = pivot_lab(y);
    z = pivot_lab(z);

    return
    {
        (116.0f * y) - 16.0f,
        500.0f * (x - y),
        200.0f * (y - z)

    };
}

RGB Lab_to_RGB(Lab_Color lab) noexcept
{
    float y = (lab.L + 16.0f) / 116.0f;
    float x = lab.a / 500.0f + y;
    float z = y - lab.b / 200.0f;

    x = inv_pivot_lab(x) * 0.95047f;
    y = inv_pivot_lab(y) * 1.00000f;
    z = inv_pivot_lab(z) * 1.08883f;

    float r = x * 3.2406f + y * -1.5372f + z * -0.4986f;
    float g = x * -0.9689f + y * 1.8758f + z * 0.0415f;
    float b = x * 0.0557f + y * -0.2040f + z * 1.0570f;

    // Gamma correction
    r = (r > 0.0031308f) ? 1.055f * powf(r, 1.0f / 2.4f) - 0.055f : 12.92f * r;
    g = (g > 0.0031308f) ? 1.055f * powf(g, 1.0f / 2.4f) - 0.055f : 12.92f * g;
    b = (b > 0.0031308f) ? 1.055f * powf(b, 1.0f / 2.4f) - 0.055f : 12.92f * b;

    return 
    {
        narrow_cast<unsigned char>(fmaxf(0.0f, fminf(1.0f, r)) * 255.0f),
        narrow_cast<unsigned char>(fmaxf(0.0f, fminf(1.0f, g)) * 255.0f),
        narrow_cast<unsigned char>(fmaxf(0.0f, fminf(1.0f, b)) * 255.0f),
        255
    };
}

Lab_Color RGB_to_OKLab(RGB c) noexcept
{
    const float r = powf(c.r / 255.0f, 2.2f);
    const float g = powf(c.g / 255.0f, 2.2f);
    const float b = powf(c.b / 255.0f, 2.2f);

    // Linear sRGB to LMS
    float l = 0.4122214708f * r + 0.5363325363f * g + 0.0514459929f * b;
    float m = 0.2119034982f * r + 0.6806995451f * g + 0.1073969566f * b;
    float s = 0.0883024619f * r + 0.2817188376f * g + 0.6299787005f * b;

    // Nonlinear transform
    l = cbrtf(l);
    m = cbrtf(m);
    s = cbrtf(s);

    return{

        0.2104542553f * l + 0.7936177850f * m - 0.0040720468f * s,
        1.9779984951f * l - 2.4285922050f * m + 0.4505937099f * s,
        0.0259040371f * l + 0.7827717662f * m - 0.8086757660f * s
    };
}

RGB OKLab_to_RGB(Lab_Color lab) noexcept
{
    float l = lab.L + 0.3963377774f * lab.a + 0.2158037573f * lab.b;
    float m = lab.L - 0.1055613458f * lab.a - 0.0638541728f * lab.b;
    float s = lab.L - 0.0894841775f * lab.a - 1.2914855480f * lab.b;

    l = l * l * l;
    m = m * m * m;
    s = s * s * s;

    float r = 4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s;
    float g = -1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s;
    float b = -0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s;

    // Clamp and convert to 0–255
    r = powf(fmaxf(0.0f, fminf(1.0f, r)), 1.0f / 2.2f);
    g = powf(fmaxf(0.0f, fminf(1.0f, g)), 1.0f / 2.2f);
    b = powf(fmaxf(0.0f, fminf(1.0f, b)), 1.0f / 2.2f);

    return 
    {
        narrow_cast<unsigned char>(r * 255.0f),
        narrow_cast<unsigned char>(g * 255.0f),
        narrow_cast<unsigned char>(b * 255.0f),
        255
    };
}

RGB HSV_to_RGB(float h, float s, float v) noexcept
{
    const float c = v * s;
    const float x = c * (1 - fabsf(fmodf(h / 60.0f, 2) - 1));
    const float m = v - c;

    float r, g, b;

    if (h < 60) { r = c; g = x; b = 0; }
    else if (h < 120) { r = x; g = c; b = 0; }
    else if (h < 180) { r = 0; g = c; b = x; }
    else if (h < 240) { r = 0; g = x; b = c; }
    else if (h < 300) { r = x; g = 0; b = c; }
    else { r = c; g = 0; b = x; }

    return RGB{
        narrow_cast<unsigned char>((r + m) * 255),
        narrow_cast<unsigned char>((g + m) * 255),
        narrow_cast<unsigned char>((b + m) * 255),
        255
    };
}

RGB HSL_to_RGB(float h, float s, float l) noexcept
{
    const float c = (1.0f - fabsf(2.0f * l - 1.0f)) * s;
    const float x = c * (1.0f - fabsf(fmodf(h / 60.0f, 2.0f) - 1.0f));
    const float m = l - c * 0.5f;

    float r, g, b;

    if (h < 60.0f) { r = c; g = x; b = 0; }
    else if (h < 120.0f) { r = x; g = c; b = 0; }
    else if (h < 180.0f) { r = 0; g = c; b = x; }
    else if (h < 240.0f) { r = 0; g = x; b = c; }
    else if (h < 300.0f) { r = x; g = 0; b = c; }
    else { r = c; g = 0; b = x; }

    return RGB{
        narrow_cast<unsigned char>((r + m) * 255.0f),
        narrow_cast<unsigned char>((g + m) * 255.0f),
        narrow_cast<unsigned char>((b + m) * 255.0f),
        255
    };
}


RGB RGB_lerp(RGB a, RGB b, float t) noexcept //depreciated
{
    return 
    {
        narrow_cast<unsigned char>(((1 - t) * a.r + t * b.r)),
        narrow_cast<unsigned char>(((1 - t) * a.g + t * b.g)),
        narrow_cast<unsigned char>(((1 - t) * a.b + t * b.b)),
        narrow_cast<unsigned char>(((1 - t) * a.a + t * b.a))
    };
}

RGB Lab_lerp(RGB a, RGB b, float t) noexcept //depreciated
{
    const Lab_Color labA = RGB_to_Lab(a);
    const Lab_Color labB = RGB_to_Lab(b);

    const Lab_Color labResult{
    (1 - t) * labA.L + t * labB.L,
    (1 - t) * labA.a + t * labB.a,
    (1 - t) * labA.b + t * labB.b
    };
    return Lab_to_RGB(labResult);
}

RGB OKlab_lerp(RGB a, RGB b, float t) noexcept
{
    const Lab_Color labA = RGB_to_OKLab(a);
    const Lab_Color labB = RGB_to_OKLab(b);

    Lab_Color result{};
    result.L = (1 - t) * labA.L + t * labB.L;
    result.a = (1 - t) * labA.a + t * labB.a;
    result.b = (1 - t) * labA.b + t * labB.b;

    return OKLab_to_RGB(result);
}




RGB HSV_lerp(RGB a, RGB b, float t) noexcept
{
    const Vector3 hsvA = ColorToHSV(a);
    const Vector3 hsvB = ColorToHSV(b);

    // Interpolera hue runt 360°
    float h1 = hsvA.x;
    float h2 = hsvB.x;
    const float delta = h2 - h1;

    if (fabsf(delta) > 180.0f) {
        if (delta > 0) h1 += 360.0f;
        else h2 += 360.0f;
    }

    const float h = fmodf((1 - t) * h1 + t * h2, 360.0f);
    const float s = (1 - t) * hsvA.y + t * hsvB.y;
    const float v = (1 - t) * hsvA.z + t * hsvB.z;

    return ColorFromHSV(h, s, v);
}

void DrawTriangleGradient(Vector2 v1, Vector2 v2, Vector2 v3, Color c1, Color c2, unsigned int resolution)
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


//float deltaE76(Lab_Color c1, Lab_Color c2) noexcept
//{
//    const float dL = c1.L - c2.L;
//    const float da = c1.a - c2.a;
//    const float db = c1.b - c2.b;
//    return sqrtf(dL * dL + da * da + db * db);
//}

Lab_Color mix_colors(const std::vector<Lab_Color>& _colors, const std::vector<float>& _weights)
{
    float L = 0, a = 0, b = 0, sum = 0;
    for (size_t i = 0; i < _colors.size(); ++i) 
    {
        L += _colors.at(i).L * _weights.at(i);
        a += _colors.at(i).a * _weights.at(i);
        b += _colors.at(i).b * _weights.at(i);
        sum += _weights.at(i);
    }

    if (sum == 0) return { 0, 0, 0 };
    return { L / sum, a / sum, b / sum };
}



RGB Huecode_to_RGB(const std::string& hue)
{
    std::map<std::string, RGB> anchors = 
    {
        {"Y", {255, 255, 0, 255}}, // gul
        {"R", {255, 0, 0, 255}},   // röd
        {"B", {0, 0, 255, 255}},   // blå
        {"G", {0, 255, 0, 255}}    // grön
    };

    std::regex re("([A-Z])([0-9]{0,2})([A-Z])");
    std::smatch match;

    if (std::regex_match(hue, match, re)) {
        std::string h1 = match[1];
        const int pct = match[2].str().empty() ? 50 : std::stoi(match[2]);
        std::string h2 = match[3];
        const RGB c1 = anchors[h1];
        const RGB c2 = anchors[h2];
        return Lab_lerp(c1, c2, pct / 100.0f);
    }

    return { 127, 127, 127, 255 }; // fallback
}

RGB blend_colors(RGB c1, RGB c2, float ratio) noexcept
{
    return Lab_lerp(c1, c2, ratio);
}

RGB NCS_To_RGB(const std::string& ncsCode)
{
    std::regex codeRe("S\\s?(\\d{2})(\\d{2})-([A-Z0-9]+)");
    std::smatch match;

    if (!std::regex_match(ncsCode, match, codeRe)) {
        return { 100, 100, 100, 255 }; // invalid
    }

    const int blackness = std::stoi(match[1]);
    const int chromaticness = std::stoi(match[2]);
    std::string hue = match[3];

    const float s = blackness / 100.0f;
    const float c = chromaticness / 100.0f;
    const float w = std::max(0.0f, 1.0f - s - c);

    const RGB colHue = Huecode_to_RGB(hue);

    // Basfärger
    const RGB white = { 255, 255, 255, 255 };
    const RGB black = { 0, 0, 0, 255 };

    // Steg 1: kulör + vit
    RGB color = blend_colors(colHue, white, w / (w + c));
    // Steg 2: lägg på svart
    color = blend_colors(color, black, s);

    return color;
}

Color_wheel::Color_wheel() :
    spokes(
    { 
    {0.0f, NCS_YELLOW},
    {PI / 2, NCS_RED},
    {PI , NCS_BLUE},
    {3 * PI / 2, NCS_GREEN} 
    })
{}

Color_wheel::Color_wheel(const std::vector<Spoke>& nodes)
{
    spokes = nodes;
    std::sort(spokes.begin(), spokes.end(), [](const Spoke& a, const Spoke& b)
        {
            return a.angle < b.angle;
        });

    // Lägg till en kopia av första färgen på slutet (för interpolering över 2π)
    if (!spokes.empty())
    {
        Spoke wrap = spokes.front();
        wrap.angle += 2 * PI;
        spokes.push_back(wrap);
    }
}

void Color_wheel::draw(Vector2 position, float radius, unsigned int resolution) const noexcept
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
            { squareSize / 2 , squareSize / 2 },
            angle * RAD2DEG,
            color
        );
    }
}

RGB Color_wheel::get_color(float radians) const noexcept
{
    radians = fmod(radians, 2 * PI);
    if (radians < 0) radians += 2 * PI;

    for (size_t i = 0; i < spokes.size() - 1; ++i)
    {
        const Spoke& a = spokes.at(i);
        const Spoke& b = spokes.at(i + 1);

        if (radians >= a.angle && radians <= b.angle)
        {
            const float t = (radians - a.angle) / (b.angle - a.angle);
            return OKlab_lerp(a.color, b.color, t);
        }
    }

    return BLACK;
}


Vector3 midpoint(Vector3 v1, Vector3 v2)
{
    return Vector3Divide(Vector3Add(v1, v2), Vector3{0.0f, 0.0f, 0.0f});
}

//+++++++++++++SOLID+++++++++++++++++

void Color_solid::Make_nodes()
{
    constexpr Vector3 center = { 0.0f, 0.0f, 0.0f };
    constexpr float SQRT3_OVER_2 = 0.866025403784f;

    const float half_height = height * 0.5f;
    const float spacing = radius / radial_steps;
    const float vertical_step = spacing * SQRT3_OVER_2;

    for (unsigned int a = 0; a < angle_steps; ++a)
    {
        const float angle_t = float(a) / angle_steps;
        const float angle = angle_t * 2.0f * PI;
        const float h = angle_t * 360.0f;

        unsigned int start_r = 1; //only make one center
        if (a == 0)
        {
            start_r = 0;
        }

        for (unsigned int r = start_r; r < radial_steps; ++r)
        {
            const float rad = (r)*spacing;
            const float s = rad / radius;

            // Number of vertical points allowed for this radius
            const float max_y =
                half_height * (1.0f - rad / radius);

            const int rows =
                static_cast<int>(max_y / vertical_step);

            for (int y = -rows; y <= rows; ++y)
            {
                const float y_pos = y * vertical_step;

                const float l = std::clamp(
                    (y_pos + half_height) / height,
                    0.0f,
                    1.0f
                );

                const Vector3 pos
                {
                    center.x + std::cos(angle) * rad,
                    center.y + y_pos,
                    center.z + std::sin(angle) * rad
                };

                const RGB color = HSL_to_RGB(h, s, l);
                color_nodes.push_back({ pos, color });

            }
        }
    }
}

void Color_solid::Draw() const
{
    for (const auto& node : color_nodes)
    {
        DrawSphere(node.position, COLOR_NODE_RADIUS, node.color);

    }

    DrawCircle3D({ 0.0f, 0.0f, 0.0f }, radius, Vector3{ 1,0,0 }, 90.0f, WHITE);
    //DrawLine3D(Bottom(), Top(), WHITE);
}



//Vector3 Color_solid::Top() const
//{
//    return Vector3Add({ 0.0f, 0.0f, 0.0f }, Vector3Scale(rotation, half_of(height)));
//   
//}

//Vector3 Color_solid::Axis_point(float normal) const
//{
//    const float factor = normal * height;
//    return Vector3Add(Bottom(), Vector3Scale(rotation, factor));
//}
//
//Vector3 Color_solid::Bottom() const
//{
//    return Vector3Subtract({ 0.0f, 0.0f, 0.0f }, Vector3Scale(rotation, half_of(height)));
//}
