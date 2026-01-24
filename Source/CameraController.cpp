#include "CameraController.h"

#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning(disable:ALL_CODE_ANALYSIS_WARNINGS)
#include "raymath.h"
#pragma warning(pop)

#include "Utilities.h"

CameraController::CameraController(Vector3& _target)
    : target(_target)
{
    camera.position = { target.x, camera_distance, camera_distance };
    camera.target = Vector3{ target };
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
    camera.fovy = 55.0f;
    camera.projection = CAMERA_CUSTOM;
}

void CameraController::Update()
{
    Mouse_drag_orbit();
    Mouse_scroll_zoom();

    const Vector2 target_angle = camera_angle;
    const float   target_distance = camera_distance;
    const Vector3 target_pos 
    {
        target.x + target_distance * cosf(target_angle.y) * sinf(target_angle.x),
        target.y + target_distance * sinf(target_angle.y),
        target.z + target_distance * cosf(target_angle.y) * cosf(target_angle.x)
    };

    if (smooth)
    {
        camera.position = animator.Smooth(camera.position, target_pos, GetFrameTime());
        camera.target = animator.Smooth(camera.target, target, GetFrameTime());
    }
    else
    {
        camera.position = target_pos;
        camera.target = target;
    }
}

void CameraController::Set_projection(CameraProjection projection)
{
    camera.projection = projection;

}

void CameraController::Begin_3D()
{
    BeginMode3D(camera);

}

void CameraController::End_3D()
{
    EndMode3D();

}

void CameraController::Orthographic_zoom()
{

}

void CameraController::Mouse_drag_orbit()
{
    if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON))
    {
        const Vector2 delta = GetMouseDelta();

        camera_angle.x -= delta.x * 0.01f;
        camera_angle.y += delta.y * 0.01f;
    }
    smooth = false;

}

void CameraController::Mouse_scroll_zoom()
{
    const float wheel = GetMouseWheelMove();
    if (wheel != 0)
    {
        camera_distance -= wheel * 1.5f;
        camera_distance = Clamp(camera_distance, 2.0f, 40.0f);
    }
    smooth = false;

}

void CameraController::Set_top_down()
{
	camera.position = { target.x, target.y + camera_distance, target.z };
	camera.target = target;
	camera.up = { 0.0f, 0.0f, -1.0f };
	camera.fovy = 45.0f;
	camera.projection = CAMERA_PERSPECTIVE;
    smooth = true;
}

void CameraController::Set_birds_eye()
{
	camera.position = { target.x, camera_distance, camera_distance };
	camera.target = Vector3{ target };
	camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
	camera.fovy = 55.0f;
	camera.projection = CAMERA_PERSPECTIVE;
    smooth = true;
}

void CameraController::Set_orbit()
{
}



void CameraController::Set_target(Vector3& _target) noexcept
{
    target = _target;
}



