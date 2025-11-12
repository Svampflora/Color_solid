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

    Lab_Color labResult{
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

//Paint_mixer::Paint Paint_mixer::blend(const std::vector<Paint> paints)
//{
//    if (paints.empty())
//        throw std::invalid_argument("för få färger att blanda angavs");
//
//    std::vector<float> blacks, chromas, amounts;
//    for (const auto& c : paints)
//    {
//        blacks.push_back(c.color.blackness);
//        chromas.push_back(c.color.chromaticness);
//        amounts.push_back(c.amount);
//    }
//
//    const float total_amount = std::accumulate(amounts.begin(), amounts.end(), 0.0f);
//
//    const float avrage_chroma = weighted_average(chromas, amounts);
//    const float avrage_black = weighted_average(blacks, amounts);
//    const float found_hue = find_hue(paints);
//
//    return Paint{ {avrage_black, avrage_chroma, found_hue}, 1, total_amount };
//}
//
//float Paint_mixer::find_hue(const std::vector<Paint> paints) const
//{
//    float sumX = 0, sumY = 0, total = 0;
//    for (size_t i = 0; i < paints.size(); ++i)
//    {
//        sumX += cos(paints.at(i).color.hue) * paints.at(i).amount * paints.at(i).color.chromaticness;
//        sumY += sin(paints.at(i).color.hue) * paints.at(i).amount * paints.at(i).color.chromaticness;
//        total += paints.at(i).amount;
//    }
//    return atan2(sumY / total, sumX / total);
//}
//
//float Paint_mixer::weighted_average(const std::vector<float> values, const std::vector<float> weights) const noexcept
//{
//    float sum = 0, total = 0;
//    for (size_t i = 0; i < values.size(); ++i)
//    {
//        sum += values.at(i) * weights.at(i);
//        total += weights.at(i);
//    }
//    return sum / total;
//}

ColorWheel::ColorWheel(const std::vector<Node>& nodes)
{
    wheel = nodes;
    std::sort(wheel.begin(), wheel.end(), [](const Node& a, const Node& b)
        {
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

void ColorWheel::draw(Vector2 position, float radius, unsigned int resolution) const noexcept
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

RGB ColorWheel::get_color(float radians) const noexcept
{
    radians = fmod(radians, 2 * PI);
    if (radians < 0) radians += 2 * PI;

    for (size_t i = 0; i < wheel.size() - 1; ++i)
    {
        const Node& a = wheel.at(i);
        const Node& b = wheel.at(i + 1);

        if (radians >= a.angle && radians <= b.angle)
        {
            const float t = (radians - a.angle) / (b.angle - a.angle);
            return OKlab_lerp(a.color, b.color, t);
        }
    }

    return BLACK;
}

NCS_Color::NCS_Color(const std::string& ncsStr)
{
    parse_from_string(ncsStr);
}

NCS_Color::NCS_Color(int _blackness, int _chromaticness, const std::string& _hueCode)
{
    blackness = std::min(_blackness, 99);
    chromaticness = std::min(_chromaticness, 99);
    hueCode = (_chromaticness == 0) ? "N" : _hueCode;
    rgb = NCS_To_RGB(to_string());
}

NCSPlusColor NCS_Color::to_NCSPlus() const
{

    return { blackness * 0.1f, chromaticness * 0.1f, hueCode_to_radians(hueCode) };
}

std::string NCS_Color::to_string() const
{
    char buf[32];
    std::snprintf(buf, sizeof(buf), "S %02d%02d-%s", blackness, chromaticness, hueCode.c_str());
    return std::string(buf);
}

void NCS_Color::draw(Vector2 position, Vector2 size) const
{
    DrawRectangleV(position, size, rgb);
    DrawTextF(this->to_string().c_str(), position.x, position.y + size.y, narrow_cast<int>(0.5f * size.y), WHITE);
}

void NCS_Color::parse_from_string(const std::string& ncsStr)
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

float NCS_Color::hueCode_to_radians(const std::string& _hueCode) const
{
    if (_hueCode == "N") return 0.0f;

    std::map<std::string, float> hueAngles = hue_Angles();
    std::regex re(R"(([A-Z]+)(\d{2})([A-Z]+))");
    std::smatch match;

    if (std::regex_match(_hueCode, match, re))
    {
        std::string base1 = match[1];
        const int percent = std::stoi(match[2]);
        std::string base2 = match[3];

        const float angle1 = hueAngles.at(base1);
        const float angle2 = hueAngles.at(base2);

        const float t = percent / 100.0f;

        // Interpolera vinklar cirkulärt
        float delta = angle2 - angle1;
        if (delta > PI) delta -= 2 * PI;
        if (delta < -PI) delta += 2 * PI;

        const float result = angle1 + delta * t;

        // Wrapa tillbaka till 0–2π
        return fmod((result + 2 * PI), (2 * PI));
    }

    // Om det är bara en basfärg
    if (hueAngles.count(_hueCode))
    {
        return hueAngles.at(_hueCode);
    }

    throw std::invalid_argument("Ogiltig hue-kod: " + _hueCode);
}

std::map<std::string, float> NCS_Color::hue_Angles() const
{
    return
    {
        {"Y", 0.0f},
        {"R", PI / 2.0f},
        {"B", PI},
        {"G", 3 * PI / 2.0f}
    };
};


void NCSTriangle::draw(Vector2 position, Vector2 size)
{
    const float cellW = size.x / resolution;
    const float cellH = size.y / resolution;


    for (unsigned int i = 0; i <= resolution; ++i)
    {
        for (unsigned int j = 0; j <= resolution - i; ++j)
        {
            const int blackness = i * (100 / resolution);
            const int chromaticness = j * (100 / resolution);

            const NCS_Color ncs = { blackness, chromaticness, hueCode };
            const Vector2 pos{ position.x + j * cellW,
                                position.y + i * cellH + (0.5f * j * cellH) };

            const float square_size = 0.5f * (size.y / resolution);

            ncs.draw(pos, { square_size, square_size });
        }
    }
}

void NCSTriangle::generateColors()
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

void ColorBicone3D::draw() const
{
    if (!initialized) return;

    const Matrix rotationMatrix = MatrixRotateXYZ({ DEG2RAD * rotation.x, DEG2RAD * rotation.y, DEG2RAD * rotation.z });

    // Modellens transform
    DrawModelEx(model, position, { 0, 1, 0 }, rotation.y, { 1, 1, 1 }, WHITE);
    //DrawModelWiresEx(model, position, { 0, 1, 0 }, rotation.y, { 1, 1, 1 }, BLACK);
}

void ColorBicone3D::buildModel()
{

    std::vector<Vector3> vertices;
    std::vector<Color> colors;

    const int segments = hue_resolution;
    const float angleStep = 2 * PI / segments;

    const float halfHeight = height * 0.5f;

    // ÖVRE HALVA: från vit → kulör (färghjul)
    for (int i = 0; i < segments; ++i)
    {
        const float angle0 = i * angleStep;
        const float angle1 = (i + 1) * angleStep;
        const Color color0 = wheel.get_color(angle0);

        for (unsigned int r = 0; r < tint_resolution; ++r)
        {
            const float t0 = static_cast<float>(r) / tint_resolution;
            const float t1 = static_cast<float>((r + 1)) / tint_resolution;

            // Vertikala nivåer
            const float y0 = Lerp(0.0f, halfHeight, t0);
            const float y1 = Lerp(0.0f, halfHeight, t1);


            const float radius0 = radius * (1.0f - t0);
            const float radius1 = radius * (1.0f - t1);

            const Vector3 v0 = { cosf(angle0) * radius0, y0, sinf(angle0) * radius0 };
            const Vector3 v1 = { cosf(angle1) * radius0, y0, sinf(angle1) * radius0 };
            const Vector3 v2 = { cosf(angle0) * radius1, y1, sinf(angle0) * radius1 };
            const Vector3 v3 = { cosf(angle1) * radius1, y1, sinf(angle1) * radius1 };

            const Color c = OKlab_lerp(color0, WHITE, t0);

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
        const float angle0 = i * angleStep;
        const float angle1 = (i + 1) * angleStep;
        const Color color0 = wheel.get_color(angle0);

        for (unsigned int r = 0; r < tint_resolution; ++r)
        {
            const float t0 = (float)r / tint_resolution;
            const float t1 = (float)(r + 1) / tint_resolution;

            const float y0 = Lerp(0.0f, -halfHeight, t0);
            const float y1 = Lerp(0.0f, -halfHeight, t1);

            const float radius0 = radius * (1.0f - t0);
            const float radius1 = radius * (1.0f - t1);

            const Vector3 v0 = { cosf(angle0) * radius0, y0, sinf(angle0) * radius0 };
            const Vector3 v1 = { cosf(angle1) * radius0, y0, sinf(angle1) * radius0 };
            const Vector3 v2 = { cosf(angle0) * radius1, y1, sinf(angle0) * radius1 };
            const Vector3 v3 = { cosf(angle1) * radius1, y1, sinf(angle1) * radius1 };

            const Color c = OKlab_lerp(color0, BLACK, t1);

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

NCSPlusColor::NCSPlusColor(float b, float c, float h)

    : blackness(std::clamp(b, 0.0f, 1.0f)),
    chromaticness(std::clamp(c, 0.0f, 1.0f)),
    hue(fmod(h, 2.0f * PI)) {}
