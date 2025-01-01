#include "precomp.h"
#include "minesweeper.h"


int main()
{
    rl::InitWindow(sst::baseX, sst::baseY, "Hello World!");
    rl::SetTargetFPS(120);
    bool fullscreen = false;
    Minesweeper game(120);
    if (!rl::IsWindowReady())
        return 0;
    
    while(!rl::WindowShouldClose())
    {
        game.UpdateState(); //Must always be before drawing
        if(rl::IsKeyPressed(rl::KEY_GRAVE))
            rl::MinimizeWindow();
        if (rl::IsKeyPressed(rl::KEY_F11))
        {
            fullscreen = !fullscreen;
            if (fullscreen) {
                rl::SetWindowSize(1920, 1080);
                rl::SetWindowPosition(rl::GetMonitorWidth(0)/2 - 1920/2, rl::GetMonitorHeight(0)/2 - 1080/2);
            }
            else {
                rl::SetWindowSize(sst::baseX, sst::baseY);
                rl::SetWindowPosition(rl::GetMonitorWidth(0)/2 - sst::baseX/2, rl::GetMonitorHeight(0)/2 - sst::baseY/2);
            }
        }
        rl::BeginDrawing();
            rl::ClearBackground(rl::LIGHTGRAY);
            game.Draw();
        rl::EndDrawing();
    }

    rl::CloseWindow();
}