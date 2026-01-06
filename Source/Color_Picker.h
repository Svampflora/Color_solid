#pragma once

//#include <ranges>
#include "State.h"
#include "ColorUtils.h"

class Color_Picker : public State
{
	//std::string ncsInput = { "S 1050-Y90R" };
	Color_wheel wheel;
	Color_solid solid;

public:
	
	Color_Picker();
	std::unique_ptr<State> Update() override;
	void Render() const override;

};