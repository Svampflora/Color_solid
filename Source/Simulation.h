#pragma once

#include "State.h"
#include "ColorUtils.h"
#include "Endscreen.h"
#include <format>
#include <array>
//------------ text utils --------------------
#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning(disable:ALL_CODE_ANALYSIS_WARNINGS)
#include "rlgl.h"


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
static inline void DrawText3D(Font font, const char* text, Vector3 position, float fontSize, float fontSpacing, float lineSpacing, bool backface, Color tint)
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
static inline void DrawQuad(std::array<Vector3, 4> corners, Color color)
{
    DrawTriangle3D(corners[0], corners[1], corners[2], color);
    DrawTriangle3D(corners[2], corners[3], corners[0], color);
}

#pragma warning(pop)

inline const char* FormatMeasurement(float meters)
{
    return (meters >= 1.0f)
        ? TextFormat("%.1f M", meters)
        : TextFormat("%.0f CM", meters * 100.0f);
}

enum class ROOM_SURFACE { None, Front, Back, Left, Right, Ceiling };

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


struct Room
{
	float width{ 4.0f }, length{ 5.0f }, height{ 2.5f};

    std::vector<Attribute> front;
    std::vector<Attribute> back;
    std::vector<Attribute> left;
    std::vector<Attribute> right;

    Rectangle Wall(ROOM_SURFACE surface) const noexcept
    {
        switch (surface)
        {
            case ROOM_SURFACE::Front:
            case ROOM_SURFACE::Back:
            {
                return{ width, height };
            } break;

            case ROOM_SURFACE::Left:
            case ROOM_SURFACE::Right:
            {
                return { length, height };
            }
            case ROOM_SURFACE::Ceiling:
            {
                return { width, length };
            }
        }
        return { 0.0f, 0.0f };
    }
	float Total_wall_area() const noexcept
	{
		const float perimeter = 2 * (width + length);
		return perimeter * height;
	}
    float Floor_area() const noexcept
    {
        return width * length;
    }
    bool Add_door(Rectangle dimensions, ROOM_SURFACE surface)
    {
        Attribute door{ dimensions.x, dimensions.y, Attribute::Type::Door };
        const float spacing = 0.5f;

        switch (surface)
        {
            case ROOM_SURFACE::Front:
            {
                float totalWidth = 0.0f;

                for (const auto& o : front)
                {
                    totalWidth += o.width + spacing;
                }

                totalWidth += dimensions.width;

                if (totalWidth + spacing * front.size() <= width)
                {
                    front.push_back(door);
                    return true;
                }
                return false;
            }
            

            case ROOM_SURFACE::Back:
            {
                //back.push_back(door);
            }

            case ROOM_SURFACE::Left:
            {
                //left.push_back(door);
            }

            case ROOM_SURFACE::Right:
            {
                //right.push_back(door);
            }
        }
        return false;
    }

	void Draw_corners(Vector3 position) const
	{
		DrawCubeWires(position, width,  height, length, WHITE);

	};
    void Draw_floor(Vector3 position, Color color) const 
    {
        DrawPlane(
            Vector3{ position.x, position.y - 0.5f * height, position.z },
            Vector2{ width, length },
            color
        );
    }
    void Draw_walls(Vector3 position, Color color) const
    {
        Draw_back_wall(position, color);
        Draw_front_wall(position, color);
        Draw_right_wall(position, color);
        Draw_left_wall(position, color);
    }
    void Draw_back_wall(Vector3 position, Color color) const
    {

        DrawQuad(std::array<Vector3, 4>{
                position.x - half_of(width), position.y - half_of(height), position.z - half_of(length) - 0.1f,
                position.x + half_of(width), position.y - half_of(height), position.z - half_of(length) - 0.1f,
                position.x + half_of(width), position.y + half_of(height), position.z - half_of(length) - 0.1f,
                position.x - half_of(width), position.y + half_of(height), position.z - half_of(length) - 0.1f
            }, color);
    }
    void Draw_front_wall(Vector3 position, Color color) const
    {

        DrawQuad(std::array<Vector3, 4>{
                position.x + half_of(width), position.y - half_of(height), position.z + half_of(length) + 0.1f,
                position.x - half_of(width), position.y - half_of(height), position.z + half_of(length) + 0.1f,
                position.x - half_of(width), position.y + half_of(height), position.z + half_of(length) + 0.1f,
                position.x + half_of(width), position.y + half_of(height), position.z + half_of(length) + 0.1f
        }, color);
    }
    void Draw_left_wall(Vector3 position, Color color) const
    {

        DrawQuad(std::array<Vector3, 4>{
                position.x - half_of(width) - 0.1f, position.y - half_of(height), position.z + half_of(length),
                position.x - half_of(width) - 0.1f, position.y - half_of(height), position.z - half_of(length),
                position.x - half_of(width) - 0.1f, position.y + half_of(height), position.z - half_of(length),
                position.x - half_of(width) - 0.1f, position.y + half_of(height), position.z + half_of(length)
        }, color);
    }
    void Draw_right_wall(Vector3 position, Color color) const
    {

        DrawQuad(std::array<Vector3, 4>{
                position.x + half_of(width) + 0.1f, position.y - half_of(height), position.z - half_of(length),
                position.x + half_of(width) + 0.1f, position.y - half_of(height), position.z + half_of(length),
                position.x + half_of(width) + 0.1f, position.y + half_of(height), position.z + half_of(length),
                position.x + half_of(width) + 0.1f, position.y + half_of(height), position.z - half_of(length)
        }, color);
        
    }
    void Draw_attributes()
    {
        //for each (auto door in front)
        //{
        //    Draw
        //}
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
    Vector3 room_position = { 0.0f,0.0f,0.0f };
    Vector2 camera_angle = { 0.0f , 0.0f };
    float camera_distance = 10.0f; // Avstånd från rummet
    unsigned int coats = 2;



    ROOM_SURFACE active_handle = ROOM_SURFACE::None;
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

        const Vector3 target = room_position;

        camera.position = 
        {
            target.x + camera_distance * cosf(camera_angle.y) * sinf(camera_angle.x),
            target.y + camera_distance * sinf(camera_angle.y),
            target.z + camera_distance * cosf(camera_angle.y) * cosf(camera_angle.x)
        };
        camera.target = target;

        UpdateRoomEditing();

		return nullptr;
	};

    void UpdateRoomEditing()
    {


        const Vector2 mouse = GetMousePosition();
        ROOM_SURFACE hovered = ROOM_SURFACE::None;


        const float y = room_position.y;

        std::vector<std::pair<ROOM_SURFACE, Vector3>> handles = {
            { ROOM_SURFACE::Front, { room_position.x, y, room_position.z + half_of(room.length) } },
            { ROOM_SURFACE::Back,  { room_position.x, y, room_position.z - half_of(room.length) } },
            { ROOM_SURFACE::Right, { room_position.x + half_of(room.width), y, room_position.z } },
            { ROOM_SURFACE::Left,  { room_position.x - half_of(room.width), y, room_position.z } },
            { ROOM_SURFACE::Ceiling, { room_position.x, y + half_of(room.height), room_position.z } },

        };

        constexpr float radius = 10.0f;

        for (auto& [handle, world_position] : handles)
        {
            const Vector2 screen_position = GetWorldToScreen(world_position, camera);
            if (CheckCollisionPointCircle(mouse, screen_position, radius))
            {
                hovered = handle;

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    active_handle = handle;
                }

                DrawCircleV(screen_position, radius, RED); 
            }
            else
            {
                DrawCircleV(screen_position, 4, GRAY);
            }
        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        {
            active_handle = ROOM_SURFACE::None;
        }

        if (active_handle != ROOM_SURFACE::None && IsMouseButtonDown(MOUSE_LEFT_BUTTON))
        {
            
            const Ray ray = GetMouseRay(GetMousePosition(), camera);
            const RayHit hit = RayIntersectPlane(ray, { 0, 1, 0 }, -room_position.y);
            const RayHit ceiling_hit = RayIntersectPlane(ray, { 1, 0, 0 }, -room_position.x);


            Vector3 hit_point = Vector3Zero();
            Vector3 ceiling_hit_point = Vector3Zero();


            if (hit.hit)
            {
                hit_point = hit.point;

            }

            if (ceiling_hit.hit)
            {
                ceiling_hit_point = ceiling_hit.point;

            }

            switch (active_handle)
            {
            case ROOM_SURFACE::Front:
            case ROOM_SURFACE::Back:
            {
                const float dist = fabsf(hit_point.z - room_position.z);
                room.length = Clamp(dist * 2.0f, min_size, max_size);
            } break;

            case ROOM_SURFACE::Left:
            case ROOM_SURFACE::Right:
            {
                const float dist = fabsf(hit_point.x - room_position.x);
                room.width = Clamp(dist * 2.0f, min_size, max_size);
            } break;

            case ROOM_SURFACE::Ceiling:
            {
                room.height = Clamp(2.0f * (ceiling_hit_point.y - room_position.y), 0.01f, 6.0f);


            } break;

            default: break;
            }
        }

        if (active_handle != ROOM_SURFACE::None && IsKeyPressed(KEY_D))
        {
            room.Add_door({ 1.0f, 2.0f }, active_handle);
        }
    }

	void Render() const override
	{
        BeginMode3D(camera);

        room.Draw_floor(room_position, DARKGRAY);
       // room.Draw_walls(room_position, PINK);
        room.Draw_corners(room_position);
        //room.Draw_attributes();

        DrawText3D(GetFontDefault(), FormatMeasurement(room.width), { room_position.x - half_of(room.width), room_position.y - half_of(room.height), room_position.y + half_of(room.length) }, 0.4f,0.1f,1.0f, true, WHITE);


        rlPushMatrix();
        rlRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        DrawText3D(GetFontDefault(), FormatMeasurement(room.length), { room_position.x - half_of(room.length), room_position.y - half_of(room.height), room_position.y + half_of(room.width) }, 0.4f, 0.1f, 1.0f, true, WHITE);
        DrawText3D(GetFontDefault(), TextFormat("%.1f M2", room.Floor_area()), { room_position.x , room_position.y - 0.49f * room.height, room_position.y }, 0.4f, 0.1f, 1.0f, true, WHITE);

        rlPopMatrix();


        rlPushMatrix();
        rlRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        DrawText3D(GetFontDefault(), FormatMeasurement(room.height), { room_position.x + half_of(room.width), room_position.y - half_of(room.length), room_position.y - half_of(room.height) }, 0.4f, 0.1f, 1.0f, true, WHITE);
        rlPopMatrix();


        EndMode3D();

        const float liters = Calculator::Liters_of_color(room, 8.0f, coats);
        DrawText(TextFormat("Beräknad färgåtgång: %.1f L", liters), 20, 40, 50, RAYWHITE);
        DrawText(TextFormat("Golvyta: %.1f M2", room.Floor_area()), 20, 80, 50, RAYWHITE);
         DrawText(TextFormat("Strykningar: %i st", coats), 20, 120, 50, RAYWHITE);


	};

};