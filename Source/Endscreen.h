#pragma once
#include "State.h"
#include <vector>

class End_screen : public State
{

public:
	std::unique_ptr<State> Update() override;
	void Render() const override;
};