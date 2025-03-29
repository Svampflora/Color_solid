#include "State.h"
#include "game.h"
#include "raymath.h"
#include <iostream>
#include <chrono>


void Game::Update()
{
    auto new_state = current_state->Update();
    if (new_state)
    {
        current_state.reset(new_state.release());
    }
}

void Game::Render() const noexcept 
{
    BeginDrawing();
    ClearBackground(BLACK);
	current_state->Render();
    EndDrawing();
}