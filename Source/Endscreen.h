#pragma once
#include"State.h"
#include <vector>


struct Score_data
{
	std::string name{ "" };
	unsigned int score{ 0 };
};

class End_screen : public State
{

public:
	std::unique_ptr<State> Update() override;
	void Render() const noexcept override;
};