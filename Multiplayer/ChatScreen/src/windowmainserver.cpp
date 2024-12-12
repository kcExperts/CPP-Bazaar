#include "window.h"
#include "screenSizeTransfer.h"

int main()
{

    rl::InitWindow(1280, 720, "SERVER");
    rl::SetTargetFPS(600);

    std::cout << std::endl << std::endl << std::endl << std::endl << std::endl << std::endl << std::endl;
    Window window;
    window.initServer();

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