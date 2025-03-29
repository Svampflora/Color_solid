#pragma once
#include <utility>

static inline float GetScreenWidthF() noexcept
{
	return static_cast<float>(GetScreenWidth());
}

static inline float GetScreenHeightF() noexcept
{
	return static_cast<float>(GetScreenHeight());
}

static inline float GetRandomValueF(int min, int max) noexcept
{
	return static_cast<float>(GetRandomValue(min, max));
}

static inline void DrawCircleF(float centerX, float centerY, float radius, Color color ) noexcept
{
	DrawCircle(static_cast<int>(centerX), static_cast<int>(centerY), radius, color);
}

static inline void DrawTextF(const char* text, float posX, float posY, int fontSize, Color color) noexcept
{
	DrawText(text, static_cast<int>(posX), static_cast<int>(posY), fontSize, color);
}

template <typename T, typename U>
inline T narrow_cast(U&& u) noexcept
{
	return static_cast<T>(std::forward<U>(u));
}

typedef struct Vector2i {
	int x;                
	int y;                
} Vector2i;
