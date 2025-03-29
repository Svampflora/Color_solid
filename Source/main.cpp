/*******************************************************************************************
*
*   raylib [core] example - Basic window
*
*   Welcome to raylib!
*
*   To test examples, just press F6 and execute raylib_compile_execute script
*   Note that compiled executable is placed in the same folder as .c file
*
*   You can find all basic examples on C:\raylib\raylib\examples folder or
*   raylib official webpage: www.raylib.com
*
*   Enjoy using raylib. :)
*
*   Example originally created with raylib 1.0, last time updated with raylib 1.0
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*b
*   Copyright (c) 2013-2022 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "game.h"
#include <iostream>





int main(void)
{    
    try
    { 
        Game game{};

        while (!WindowShouldClose())
        {
            game.Update();
            game.Render();
        }
    }
    catch (const std::exception& _exception)
    {
        std::cerr << "Exception caught: " << _exception.what() << std::endl;
    }

    return 0;
}