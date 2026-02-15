#pragma once
#include "Room.h"
#include <vector>

struct Project
{
	Room room{};
	std::vector<Paint> paints;

	Project() noexcept
	{
		paints.push_back(Paint({ 250, 150, 150, 255 }));
		paints.push_back(Paint({ 237, 237, 213, 255 }));
		paints.push_back(Paint({ 66, 95, 150, 255 }));
	}
};
