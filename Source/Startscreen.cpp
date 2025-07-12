#include "Startscreen.h"
#include "Gameplay.h"
#include "Simulation.h"

Start_screen::Start_screen() noexcept{}


std::unique_ptr<State> Start_screen::Update()
{
	if (IsKeyReleased(KEY_SPACE))
	{
		return std::make_unique<Simulation>();
	}
	return nullptr;
}

void Start_screen::Render() const
{
	Draw_title();
}

void Start_screen::Draw_title() const noexcept
{
	DrawText("Color Solid", 200, 100, 160, MAGENTA);
	DrawText("PRESS SPACE TO BEGIN", 200, 350, 40, WHITE);
}