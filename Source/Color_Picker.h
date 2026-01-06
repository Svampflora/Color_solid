#pragma once

//#include <ranges>
#include "Menu.h"
#include "ColorUtils.h"

class Color_picker
{
	Color_solid solid;
	int hovered = -1;
public:
	
	Color_picker();
	int  Hovered_index() const noexcept { return hovered; }
	void Update(const Camera& camera);
	void Draw() const;

};