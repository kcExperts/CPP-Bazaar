#include "window.h"
#include "screenSizeTransfer.h"

int main()
{

    rl::InitWindow(1280, 720, "CLIENT");
    rl::SetTargetFPS(600);

    Window window;
    window.initClient();

    while(!rl::WindowShouldClose())
    {
        
        rl::BeginDrawing();
            rl::ClearBackground(rl::RAYWHITE);
            window.draw();

        rl::EndDrawing();
        window.updateLogic();
    }

    rl::CloseWindow();

    
    return 0;
}