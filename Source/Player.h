#pragma once

#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning(disable:ALL_CODE_ANALYSIS_WARNINGS)
#include "raylib.h"
#pragma warning(pop)

#include "Utilities.h"


static constexpr int start_lives = 3;

class Player
{
	

public:

	void Update() noexcept;
	void Render() const noexcept;
};