#pragma once

#include <ranges>


#include "State.h"
#include "Player.h"

class Play_screen : public State
{
	Player player{};
	int score{0};


public:
	
	Play_screen();
	std::unique_ptr<State> Update() override;
	void Render() const noexcept override;
};