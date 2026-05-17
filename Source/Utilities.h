#pragma once

#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning(disable:ALL_CODE_ANALYSIS_WARNINGS)
#include "raylib.h"
#pragma warning(pop)

#include <type_traits>

[[nodiscard]] constexpr inline float half_of(float whole) noexcept {
	return 0.5f * whole;
}

namespace util 
{
    [[nodiscard]] constexpr inline float clamp(float x, float min, float max) noexcept {
        return (x < min) ? min : (x > max ? max : x);
    }

    [[nodiscard]] constexpr inline float in_radians(float degrees) noexcept {
        return degrees * static_cast<float>(PI / 180.0);
    }

    [[nodiscard]] constexpr inline float in_degrees(float radians) noexcept {
        return radians * static_cast<float>(180.0 / PI);
    }

    // Vector helpers
    template<typename T>
    [[nodiscard]] constexpr inline T lerp(T a, T b, float t) noexcept {
        return a + (b - a) * t;
    }

}

float GetScreenWidthF() noexcept;
float GetScreenHeightF() noexcept;
float GetRandomValueF(int min, int max) noexcept;
void DrawCircleF(float centerX, float centerY, float radius, Color color ) noexcept;
void DrawTextF(const char* text, float posX, float posY, int fontSize, Color color) noexcept;
Vector2 GetScreenCenter() noexcept;
constexpr int Clamp(int value, int min, int max) noexcept;
void DecrementClampZero(size_t& value) noexcept;


template <typename T, typename U>
inline T narrow_cast(U&& u) noexcept
{
	return static_cast<T>(std::forward<U>(u));
}

typedef struct Vector2i {
	int x;                
	int y;                
} Vector2i;




