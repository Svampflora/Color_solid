#pragma once
#include "Animator.h"

#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning(disable:ALL_CODE_ANALYSIS_WARNINGS)
#include "raylib.h"
#pragma warning(pop)

class CameraController 
{
    Animator animator{};
    Vector3& target;
    Vector2 camera_angle = { 0.0f , 0.0f };
    float camera_distance = 10.0f;

public:
    Camera3D camera{};
    bool smooth = false;


    CameraController(Vector3& _target);
    void Set_top_down();
    void Set_birds_eye();
    void Set_orbit();
    void Set_target(Vector3& _target) noexcept;
    void Update();
    void Update_zoom();
    void Set_projection(CameraProjection projection);

    void Begin_3D();
    void End_3D();

    void Orthographic_zoom();

private:
    void Mouse_scroll_zoom();
    void Mouse_drag_orbit();
    
};
