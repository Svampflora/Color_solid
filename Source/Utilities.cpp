#include "Utilities.h"

#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning(disable:ALL_CODE_ANALYSIS_WARNINGS)
#include "raymath.h"
#pragma warning(pop)


float GetScreenWidthF() noexcept
{
	return static_cast<float>(GetScreenWidth());
}

float GetScreenHeightF() noexcept
{
	return static_cast<float>(GetScreenHeight());
}

float GetRandomValueF(int min, int max) noexcept
{
	return static_cast<float>(GetRandomValue(min, max));
}

void DrawCircleF(float centerX, float centerY, float radius, Color color) noexcept
{
	DrawCircle(static_cast<int>(centerX), static_cast<int>(centerY), radius, color);
}

void DrawTextF(const char* text, float posX, float posY, int fontSize, Color color) noexcept
{
	DrawText(text, static_cast<int>(posX), static_cast<int>(posY), fontSize, color);
}

Vector2 GetScreenCenter() noexcept
{
	return { GetScreenWidthF(), GetScreenHeightF() };
}


