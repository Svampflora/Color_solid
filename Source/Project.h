#pragma once
#include "Room.h"
#include <vector>

struct Project
{
	Room room{};
	std::vector<Paint> paints;

	Project() noexcept
	{
		Add_paint({ 250, 150, 150, 255 });
		Add_paint({ 237, 237, 213, 255 });
		Add_paint({ 66, 95, 150, 255 });
	}

	void Add_paint(Color color) noexcept
	{
		Paint paint(color);
		paint.name = "Färg " + std::to_string(paints.size() + 2);
		paints.push_back(paint);
	}
};
