#pragma once
#include"State.h"

class Start_screen : public State
{
	void Draw_title() const noexcept;
public:
	Start_screen() noexcept;
	std::unique_ptr<State> Update() override;
	void Render() const noexcept override;
};
