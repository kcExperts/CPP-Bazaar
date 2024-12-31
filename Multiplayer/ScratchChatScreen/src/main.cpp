#include "precomp.h"
#include "ChatModule.h"
#include <filesystem>
#include <iostream>
 
int main() {
    rl::InitWindow(sst::baseX, sst::baseY, "Hello World!");
    bool fullscreen = false;;
    rl::SetTargetFPS(120);
    ChatModule test(120);
    
    if (!rl::IsWindowReady())
        return 0;
    
    while(!rl::WindowShouldClose())
    {
        test.UpdateState(); //Must always be before drawing
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
            rl::ClearBackground(rl::RAYWHITE);

            test.Draw();

            rl::DrawText(rl::TextFormat("FPS: %i", rl::GetFPS()), 100, 100, 10, rl::RED);

        rl::EndDrawing();
    }

    rl::CloseWindow();

}