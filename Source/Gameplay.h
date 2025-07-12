#pragma once

//#include <ranges>
#include "State.h"
#include "ColorUtils.h"

class Play_screen : public State
{
	std::string ncsInput = { "S 1050-Y90R" };
	ColorWheel wheel;
	ColorBicone3D solid;

public:
	
	Play_screen();
	std::unique_ptr<State> Update() override;
	void Render() const override;

};