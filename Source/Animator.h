#pragma once

#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning(disable:ALL_CODE_ANALYSIS_WARNINGS)
#include "raylib.h"
#pragma warning(pop)

class Animator
{
public:
    float damping = 10.0f;
    
    Vector3 Smooth(const Vector3& current, const Vector3& target, float dt) const noexcept;
    Vector2 Smooth(const Vector2& current, const Vector2& target, float dt) const noexcept;
    float   Smooth(const float& current, const float& target, float dt) const noexcept;
};