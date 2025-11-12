#include "Application.h"

#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning(disable:ALL_CODE_ANALYSIS_WARNINGS)
#include "raylib.h"
#pragma warning(pop)

#include "State.h"
#include <iostream>
#include <chrono>


void Application::Update()
{
    auto new_state = current_state->Update();
    if (new_state)
    {
        current_state.reset(new_state.release());
    }
}

void Application::Render() const 
{
    BeginDrawing();
    ClearBackground(BLACK);


	current_state->Render();


    EndDrawing();
}