#include "precomp.h"
#include "ChatModule.h"
#include <filesystem>
#include <iostream>
 
int main() {
    rl::InitWindow(sst::baseX, sst::baseY, "Hello World!");
    rl::SetTargetFPS(60);
    ChatModule test;
    
    if (!rl::IsWindowReady())
        return 0;
    
    while(!rl::WindowShouldClose())
    {
        test.UpdateState();
        if(rl::IsKeyPressed(rl::KEY_A))
            rl::MinimizeWindow();

        if (rl::IsKeyPressed(rl::KEY_B))
            rl::RestoreWindow();

        rl::BeginDrawing();
            rl::ClearBackground(rl::RAYWHITE);

            test.Draw();

        rl::EndDrawing();
    }

    rl::CloseWindow();

}