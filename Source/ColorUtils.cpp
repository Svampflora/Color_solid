#include "ColorUtils.h"
#include "Utilities.h"
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

    return{
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

    return {
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

RGB RGB_lerp(RGB a, RGB b, float t) noexcept
{
    return 
    {
        narrow_cast<unsigned char>(((1 - t) * a.r + t * b.r)),
        narrow_cast<unsigned char>(((1 - t) * a.g + t * b.g)),
        narrow_cast<unsigned char>(((1 - t) * a.b + t * b.b)),
        narrow_cast<unsigned char>(((1 - t) * a.a + t * b.a))
    };
}

RGB Lab_lerp(RGB a, RGB b, float t) noexcept
{
    const Lab_Color labA = RGB_to_Lab(a);
    const Lab_Color labB = RGB_to_Lab(b);

    Lab_Color labResult;
    labResult.L = (1 - t) * labA.L + t * labB.L;
    labResult.a = (1 - t) * labA.a + t * labB.a;
    labResult.b = (1 - t) * labA.b + t * labB.b;

    return Lab_to_RGB(labResult);
}

RGB OKlab_lerp(RGB a, RGB b, float t) noexcept
{
    const Lab_Color labA = RGB_to_OKLab(a);
    const Lab_Color labB = RGB_to_OKLab(b);

    Lab_Color result;
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

//float deltaE76(Lab_Color c1, Lab_Color c2) noexcept
//{
//    const float dL = c1.L - c2.L;
//    const float da = c1.a - c2.a;
//    const float db = c1.b - c2.b;
//    return sqrtf(dL * dL + da * da + db * db);
//}

Lab_Color mix_colors(const std::vector<Lab_Color>& _colors, const std::vector<float>& _weights) noexcept
{
    float L = 0, a = 0, b = 0, sum = 0;
    for (size_t i = 0; i < _colors.size(); ++i) 
    {
        L += _colors[i].L * _weights[i];
        a += _colors[i].a * _weights[i];
        b += _colors[i].b * _weights[i];
        sum += _weights[i];
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