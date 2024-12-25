#include "precomp.h"
#include "ChatModule.h"
#include <filesystem>
#include <iostream>
 
int main() {
    rl::InitWindow(sst::baseX, sst::baseY, "Hello World!");
    rl::SetTargetFPS(120);
    ChatModule test;
    
    if (!rl::IsWindowReady())
        return 0;
    
    while(!rl::WindowShouldClose())
    {
        test.UpdateState(); //Must always be before drawing
        if(rl::IsKeyPressed(rl::KEY_GRAVE))
            rl::MinimizeWindow();

        rl::BeginDrawing();
            rl::ClearBackground(rl::RAYWHITE);

            test.Draw();

            rl::DrawText(rl::TextFormat("FPS: %i", rl::GetFPS()), 100, 100, 10, rl::RED);

        rl::EndDrawing();
    }

    rl::CloseWindow();

}