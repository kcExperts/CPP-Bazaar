#include "program.h"
#include <stdexcept>

Program::Program()
{
    //Initialize WSA
    int wsaerr;
    WORD wVersionRequested = MAKEWORD(2,2); //Want version 2.2 of winsock2, which is the most recent one
    wsaerr = WSAStartup(wVersionRequested, &wsadata);
    if (wsaerr != 0)
    {
        throw std::runtime_error("WSA V2.2 not found.");
    }
    isServer = false;
    isClient = false;
    end = false;
    isMenuInitialized = false;
    debug = false;
    server = nullptr;
    client = nullptr;
    menu = mainMenu;
    mouse = Mouse();
    //Initialize Raylib things
    rl::SetConfigFlags(rl::FLAG_VSYNC_HINT); //Prevents screen tearing
    rl::InitWindow(sst::baseX, sst::baseY, "Chat With Friends");
    rl::SetTargetFPS(FPS);
}

void Program::loop()
{
    while (!end)
    {
        if (rl::WindowShouldClose())
            end = true;

        if (!isMenuInitialized)
            initializeMenu();

        if (rl::IsKeyPressed(rl::KEY_TAB))
            debug = !debug;
        
        rl::BeginDrawing();
            rl::ClearBackground(rl::GRAY);
            draw();
            if(debug)
                drawDebug();
        rl::EndDrawing();
        //Logic updates
        mouse.updateMousePosition();
    }
}

void Program::close()
{
    //Add destructor to each of these
    server = nullptr;
    client = nullptr;
    WSACleanup();
    rl::CloseWindow();
}

void Program::updateLogic(ChatEvents state)
{
    switch (state)
    {
    case doNothing:
        break;
    case openClientJoinMenu:
        menu = clientJoinMenu;
        isMenuInitialized = false;
        break;
    case openHostSettingsMenu:
        menu = hostSettingsMenu;
        isMenuInitialized = false;
        break;
    case 3:
        break;
    default:
        break;
    }
}

ChatEvents Program::getStateTransition()
{
    switch (menu)
    {
    case 0:
        return getMainMenuState();
        break;
    case 1:
        
        break;
    case 2:
        
        break;
    case 3:
        
        break;
    case 4:
        
        break;
    default:
        break;
    }
}


void Program::draw()
{
    switch (menu)
    {
    case 0:
        drawMainMenu();
        break;
    case 1:
        drawClientJoinMenu();
        break;
    case 2:
        drawHostSettingsMenu();
        break;
    case 3:
        drawHostMain();
        break;
    case 4:
        drawClientMain();
        break;
    default:
        break;
    }
}

void Program::initializeMenu()
{
    std::string text;
    int font;
    switch (menu)
    {
    case 0:
            font = 50;
            text = "Host";
            addButton(text, font, centerTextX(text, font), centerTextY(text.c_str(), font) - (10 + font));
            text = "Join";
            addButton(text, font, centerTextX(text, font), centerTextY(text.c_str(), font) + (10 + font));
        break;
    case 1:
        
        break;
    case 2:
        
        break;
    case 3:
        
        break;
    case 4:
        
        break;
    default:
        break;
    }
    isMenuInitialized = true;
}

void Program::drawMainMenu()
{
    int font = 50;
    std::string text = "Host";
    rl::DrawText(text.c_str(), sst::cx(centerTextX(text, font)), sst::cy(centerTextY(text.c_str(), font)) - sst::cx(10 + font), sst::cx(font), buttons[0].isHovered(mouse) ? rl::RED : rl::BLACK);
    text = "Join";
    rl::DrawText(text.c_str(), sst::cx(centerTextX(text, font)), sst::cy(centerTextY(text.c_str(), font)) + sst::cx(10 + font),  sst::cx(font), buttons[1].isHovered(mouse) ? rl::RED : rl::BLACK);
}

void Program::drawClientJoinMenu()
{

}

void Program::drawHostSettingsMenu()
{

}

void Program::drawHostMain()
{
    
}

void Program::drawClientMain()
{

}

void Program::drawDebug()
{
    //Draw mouse hitbox
    rl::DrawRectangleRec(mouse.mouseHitbox(), rl::PURPLE);
    //Draw button hitboxes
    for (int i = 0; i < buttons.size(); i++)
    {
        rl::DrawRectangleLinesEx(buttons[i].getBounds(), 5, rl::PURPLE);
    }
    //Show current selection
    rl::DrawText(rl::TextFormat("Buttons[%i]", buttonHovered()), 0, 0, sst::cx(20), rl::BLACK);
}

ChatEvents Program::getMainMenuState()
{
    if (rl::IsMouseButtonPressed(rl::MOUSE_BUTTON_LEFT))
    {
        switch (buttonHovered())
        {
        case MMS_HOST:
            return openHostSettingsMenu;
            break;
        case MMS_JOIN:
            return openClientJoinMenu;
            break;
        }
    }
    return doNothing;
}

//Following functions were recycled from another project
void Program::addButton(const std::string& label, int font, int posX, int posY)
{
    buttons.push_back(Button(label, font, posX, posY));
}

void Program::addButton(const std::string& label, rl::Rectangle rec)
{
    buttons.push_back(Button(label ,rec));
}

int Program::centerTextX(const std::string& text, int font)
{
    return (sst::baseX/2) - (rl::MeasureText(text.c_str(), (font))/2);
}

int Program::centerTextY(const std::string& text, int font)
{
    return (sst::baseY/2);
}

int Program::buttonHovered() const
{
    for(int i = 0; i < buttons.size(); i++)
    {
        if (buttons[i].isHovered(mouse))
            return i;
    }
    return -1;
}