#pragma once

#include "Menu.h"
#include "ColorUtils.h"

//const float stiffness = 12.0f;
//const float damping = 8.0f;

constexpr float STIFFNESS = 0.005f;
constexpr float DAMPING = 0.5f;

class Color_picker
{
public:
	Vector3 position;
private:
	Color_solid solid;
	Quaternion rotation;
	Quaternion target_rotation;
	Vector3 angular_velocity;
	int hovered = -1;
public:
	
	Color_picker();
	RGB Get_color();
	int  Hovered_index() const noexcept { return hovered; }
	void Update(const Camera& camera);
	void Draw() const;

private:
	void Mouse_rotation(const Camera& camera);
	void Node_selection(const Camera& camera);

};