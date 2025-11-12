#include "Endscreen.h"

#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning(disable:ALL_CODE_ANALYSIS_WARNINGS)
#include "raylib.h"
#pragma warning(pop)

#include "Startscreen.h"
#include <fstream>
#include <iostream>


std::unique_ptr<State> End_screen::Update()
{


	if (IsKeyReleased(KEY_ENTER))
	{
		return std::make_unique<Start_screen>();
	}
	return nullptr;
}

void End_screen::Render() const
{

}

