#include "Animator.h"

#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning(disable:ALL_CODE_ANALYSIS_WARNINGS)
#include "raymath.h"
#pragma warning(pop)

#include "Utilities.h"

Vector3 Animator::Smooth(const Vector3& current, const Vector3& target, float dt) const noexcept
{
    const float k = damping * dt;
    return 
    {
        current.x + (target.x - current.x) * k,
        current.y + (target.y - current.y) * k,
        current.z + (target.z - current.z) * k
    };
}

Vector2 Animator::Smooth(const Vector2& current, const Vector2& target, float dt) const noexcept
{
    const float k = damping * dt;
    return 
    {
        current.x + (target.x - current.x) * k,
        current.y + (target.y - current.y) * k
    };
}

float Animator::Smooth(const float& current, const float& target, float dt) const noexcept
{
    return current + (target - current) * (damping * dt);
}