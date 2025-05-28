#include "ColorUtils.h"
#include "Utilities.h"
#include <cmath>
#include <map>


Lab_Color RGB_to_Lab(Color color) noexcept
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

    Lab_Color lab;
    lab.L = (116.0f * y) - 16.0f;
    lab.a = 500.0f * (x - y);
    lab.b = 200.0f * (y - z);

    return lab;
}

Color Lab_to_RGB(Lab_Color lab) noexcept
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

    Color result = {
        (unsigned char)(fmaxf(0.0f, fminf(1.0f, r)) * 255.0f),
        (unsigned char)(fmaxf(0.0f, fminf(1.0f, g)) * 255.0f),
        (unsigned char)(fmaxf(0.0f, fminf(1.0f, b)) * 255.0f),
        255
    };
    return result;
}

Color RGB_lerp(Color a, Color b, float t) noexcept
{
    Color result;
    result.r = narrow_cast<unsigned char>(((1 - t) * a.r + t * b.r));
    result.g = narrow_cast<unsigned char>(((1 - t) * a.g + t * b.g));
    result.b = narrow_cast<unsigned char>(((1 - t) * a.b + t * b.b));
    result.a = narrow_cast<unsigned char>(((1 - t) * a.a + t * b.a));
    return result;
}

Color Lab_lerp(Color a, Color b, float t) noexcept
{
    Lab_Color labA = RGB_to_Lab(a);
    Lab_Color labB = RGB_to_Lab(b);

    Lab_Color labResult;
    labResult.L = (1 - t) * labA.L + t * labB.L;
    labResult.a = (1 - t) * labA.a + t * labB.a;
    labResult.b = (1 - t) * labA.b + t * labB.b;

    return Lab_to_RGB(labResult);
}

Color HSV_lerp(Color a, Color b, float t) noexcept
{
    Vector3 hsvA = ColorToHSV(a);
    Vector3 hsvB = ColorToHSV(b);

    // Interpolera hue runt 360°
    float h1 = hsvA.x;
    float h2 = hsvB.x;
    float delta = h2 - h1;

    if (fabsf(delta) > 180.0f) {
        if (delta > 0) h1 += 360.0f;
        else h2 += 360.0f;
    }

    float h = fmodf((1 - t) * h1 + t * h2, 360.0f);
    float s = (1 - t) * hsvA.y + t * hsvB.y;
    float v = (1 - t) * hsvA.z + t * hsvB.z;

    return ColorFromHSV(h, s, v);
}

float deltaE76(Lab_Color c1, Lab_Color c2) 
{
    const float dL = c1.L - c2.L;
    const float da = c1.a - c2.a;
    const float db = c1.b - c2.b;
    return sqrtf(dL * dL + da * da + db * db);
}

Lab_Color blendColors(const std::vector<Lab_Color>& colors, const std::vector<float>& weights) 
{
    float L = 0, a = 0, b = 0, sum = 0;
    for (size_t i = 0; i < colors.size(); ++i) 
    {
        L += colors[i].L * weights[i];
        a += colors[i].a * weights[i];
        b += colors[i].b * weights[i];
        sum += weights[i];
    }

    if (sum == 0) return { 0, 0, 0 };
    return { L / sum, a / sum, b / sum };
}



Color HueToColor(const std::string& hue) 
{
    std::map<std::string, Color> anchors = {
        {"Y", {255, 255, 0, 255}}, // gul
        {"R", {255, 0, 0, 255}},   // röd
        {"B", {0, 0, 255, 255}},   // blå
        {"G", {0, 255, 0, 255}}    // grön
    };

    std::regex re("([A-Z])([0-9]{0,2})([A-Z])");
    std::smatch match;

    if (std::regex_match(hue, match, re)) {
        std::string h1 = match[1];
        int pct = match[2].str().empty() ? 50 : std::stoi(match[2]);
        std::string h2 = match[3];
        Color c1 = anchors[h1];
        Color c2 = anchors[h2];
        return Lab_lerp(c1, c2, pct / 100.0f);
    }

    return { 127, 127, 127, 255 }; // fallback
}

Color BlendColors(Color c1, Color c2, float ratio) 
{
    return Lab_lerp(c1, c2, ratio);
}

Color NCS_To_RGB(const std::string& ncsCode) 
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

    const Color colHue = HueToColor(hue);

    // Basfärger
    const Color white = { 255, 255, 255, 255 };
    const Color black = { 0, 0, 0, 255 };

    // Steg 1: kulör + vit
    Color color = BlendColors(colHue, white, w / (w + c));
    // Steg 2: lägg på svart
    color = BlendColors(color, black, s);

    return color;
}