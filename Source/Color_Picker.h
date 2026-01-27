#pragma once

#include "Menu.h"
#include "ColorUtils.h"

constexpr float ROTATION_SENSATIVITY = 0.05f;
constexpr float DAMPING = 0.99f;

class Color_picker
{
	Color_solid solid;
	Quaternion rotation;
	Quaternion target_rotation;
	Vector3 position;
	Vector3 angular_velocity;
	int hovered = -1;
public:
	
	Color_picker();
	int  Hovered_index() const noexcept { return hovered; }
	void Update(const Camera& camera);
	void Draw() const;

private:
	void Mouse_rotation();
	void Node_selection(const Camera& camera);

};