#pragma once

#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning(disable:ALL_CODE_ANALYSIS_WARNINGS)
#include "raylib.h"
#pragma warning(pop)

#include "State.h"
#include "Room.h"

struct Camera3D;

class Editor : public State
{
	Camera3D camera = { 0 };
    Room room{};
    Vector2 camera_angle = { 0.0f , 0.0f };
    float camera_distance = 10.0f;
    Wall::Handle handle{};
    std::vector<Paint> paints;

    Paint* selected_paint = nullptr;


    float min_size = 1.0f;
    float max_size = 10.0f;

    Wall* Hovered_handle();
    Wall* Hovered_wall();
    const Wall* Hovered_wall() const;
    void Edit();
    void Paint_selection() noexcept;

public:

    Editor() noexcept;
    std::unique_ptr<State> Update() override;
    void Render() const override;
};