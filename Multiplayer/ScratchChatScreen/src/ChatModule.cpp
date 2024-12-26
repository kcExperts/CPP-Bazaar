#include "ChatModule.h"

ChatModule::ChatModule()
{
    ChatWindow = rl::Rectangle{CHATWINDOW_START_X, CHATWINDOW_START_Y, CHATWINDOW_WIDTH, CHATWINDOW_HEIGHT};
    WindowColor = rl::BLACK;
    currentMenu = Init;
    InitLoadingImage();
    areButtonsInitialized = false;
    usernameLength = 0;
    messageLength = 0;
    portLength = 0;
    ipLength = 0;
    isTyping = false;
}

ChatModule::~ChatModule() {};

void ChatModule::Draw()
{
    rl::DrawRectangleRec(ChatWindow, WindowColor);
    switch (currentMenu)
    {
    case Init:
        if (!areButtonsInitialized) InitializeInit();
        DrawInit();
        break;
    case Select:
        if (!areButtonsInitialized) InitializeSelect();
        DrawSelect();
        break;
    case Settings:
        //Settings
        break;
    case Host:
        if (!areButtonsInitialized) InitializeHost();
        DrawHost();
        break;
    case Join:
        //Join
        break;
    case Loading:
        DrawLoading();
        break;
    }
}

void ChatModule::InitializeHost()
{
    buttons.clear();
    areButtonsInitialized = true;
    ButtonInfoInit();
    buttonInfo.hasTextBox = false;
    buttonInfo.locationOrientation = Left;
    buttonInfo.text = "Back";
    buttonInfo.textLocationPoint = {CHATWINDOW_TEXT_RIGHT_X, CHATWINDOW_TEXT_TOP_Y};
    buttons["Back"] = buttonInfo;
    buttonInfo.text = "Create";
    buttonInfo.textLocationPoint.y = CHATWINDOW_TEXT_BOTTOMRIGHT_Y;
    buttons["Create"] = buttonInfo;
    buttonInfo.locationOrientation = Center;
    buttonInfo.hasTextBox = true;
    rl::Rectangle rec = CenterRect(CHATWINDOW_USERNAMEBOX_WIDTH, CHATWINDOW_USERNAMEBOX_HEIGHT);
    rec.y -= 20;
    buttonInfo.textBox.location = rec;
    buttonInfo.textBox.type = Username;
    buttonInfo.text = "Insert Username Below";
    buttons["Userbox"] = buttonInfo;
    buttonInfo.textBox.location.y += 75;
    buttonInfo.textBox.type = Port;
    buttonInfo.text = "Insert Port Number Below";
    buttons["Portbox"] = buttonInfo;
}

//Requires username and port
void ChatModule::DrawHost() const
{
    DrawTextAboveRect(buttons.at("Userbox"), 10);
    DrawTextAboveRect(buttons.at("Portbox"), 10);
    DrawTextLeftOfPoint(buttons.at("Back"));
    DrawTextLeftOfPoint(buttons.at("Create"));
    DrawTextBox(buttons.at("Userbox"), Username, true);
    DrawTextBox(buttons.at("Portbox"), Port, true);
}

void ChatModule::UpdateHost()
{
    ModifyTextBox();
    CheckButtonCollision();
    if (rl::IsMouseButtonPressed(rl::MOUSE_BUTTON_LEFT))
    {
        bool isE = buttons.at("Userbox").textBox.isEditing;
        if (buttons.at("Userbox").isMouseHovering && ((!isTyping && !isE) || (isTyping && isE)))
        {
            buttons.at("Userbox").textBox.isEditing = !buttons.at("Userbox").textBox.isEditing;
            isTyping = !isTyping;
        }
        isE = buttons.at("Portbox").textBox.isEditing;
        if (buttons.at("Portbox").isMouseHovering && ((!isTyping && !isE) || (isTyping && isE)))
        {
            buttons.at("Portbox").textBox.isEditing = !buttons.at("Portbox").textBox.isEditing;
            isTyping = !isTyping;
        }
        if (buttons.at("Back").isMouseHovering && !isTyping)
        {
            areButtonsInitialized = false;
            currentMenu = Select;
        }
    }
}

void ChatModule::InitializeSelect()
{
    buttons.clear();
    areButtonsInitialized = true;
    ButtonInfoInit();
    buttonInfo.text = "Host";
    buttonInfo.textLocationPoint = {GetCenterX(), GetCenterY() - 50};
    buttons["Host"] = buttonInfo;
    buttonInfo.textLocationPoint.y += 50;
    buttonInfo.text = "Join";
    buttons["Join"] = buttonInfo;
    buttonInfo.textLocationPoint.y += 50;
    buttonInfo.text = "Settings";
    buttons["Settings"] = buttonInfo;
}

void ChatModule::DrawSelect() const
{
    DrawTextOnPoint(buttons.at("Host"));
    DrawTextOnPoint(buttons.at("Join"));
    DrawTextOnPoint(buttons.at("Settings"));
}

void ChatModule::UpdateSelect()
{
    //Verify if any button is being hovered over
    CheckButtonCollision();
    if (rl::IsMouseButtonPressed(rl::MOUSE_BUTTON_LEFT))
    {
        if (buttons.at("Host").isMouseHovering)
        {
            areButtonsInitialized = false;
            currentMenu = Host;
        }
        if (buttons.at("Join").isMouseHovering)
        {
            areButtonsInitialized = false;
            currentMenu = Join;
        }
        if (buttons.at("Settings").isMouseHovering)
        {
            areButtonsInitialized = false;
            currentMenu = Settings;
        }
    }
}

void ChatModule::DrawLoading() const
{
    // What the fuck is going on with the line below
    rl::Rectangle loadingDest = {GetCenterX() + T_loading.textureCenter.x - 100 / 2 + ChatWindow.x, GetCenterY() + T_loading.textureCenter.y - 100 / 2 + ChatWindow.y, 100, 100};
    rl::DrawTexturePro(T_loading.texture, T_loading.sourceRec, loadingDest, T_loading.textureCenter, T_loading.currentAngle, rl::WHITE);
}

void ChatModule::UpdateLoading()
{
    T_loading.framesCounted++;
    if (T_loading.framesCounted >= T_loading.framesUntilUpdate)
    {
        T_loading.framesCounted = 0;
        T_loading.currentAngle += 30.0f;
        T_loading.currentAnimationStage++;
        if (T_loading.currentAnimationStage >= T_loading.animationStages)
            T_loading.currentAngle = 0.0f;
        T_loading.currentAnimationStage = 0;
    }
}

void ChatModule::InitializeInit()
{
    buttons.clear();
    areButtonsInitialized = true;
    ButtonInfoInit();
    buttonInfo.textLocationPoint = {GetCenterX(), GetCenterY() - 50};
    buttonInfo.text = "Welcome to the Chat Module!";
    buttons["Greeting"] = buttonInfo;
    buttonInfo.textLocationPoint.y += 100;
    buttonInfo.text = "Click anywhere inside to begin";
    buttons["Begin"] = buttonInfo;
}

void ChatModule::DrawInit() const
{
    DrawTextOnPoint(buttons.at("Greeting"));
    DrawTextOnPoint(buttons.at("Begin"));
}

void ChatModule::UpdateInit()
{
    if (rl::IsMouseButtonPressed(rl::MOUSE_BUTTON_LEFT) && rl::CheckCollisionPointRec(rl::GetMousePosition(), ChatWindow))
    {
        currentMenu = Select;
        areButtonsInitialized = false;
    }
}

void ChatModule::UpdateState()
{
    Drag();
    switch (currentMenu)
    {
    case Init:
        UpdateInit();
        break;
    case Select:
        UpdateSelect();
        break;
    case Settings:
        //Settings
        break;
    case Host:
        UpdateHost();
        break;
    case Join:
        //Join
        break;
    case Loading:
        UpdateLoading();
        break;
    }
}

void ChatModule::Drag()
{
    rl::Vector2 mousePos = rl::GetMousePosition();

    // Store the current position of the mouse during a left click to use as the reference point for the starting drag position
    if (rl::IsMouseButtonPressed(rl::MOUSE_BUTTON_LEFT))
    {
        if (rl::CheckCollisionPointRec(mousePos, ChatWindow))
            LeftClickPos = (rl::Vector2){mousePos.x - ChatWindow.x, mousePos.y - ChatWindow.y};
    }

    // Ensure that the user is dragging the chatwindow
    if (rl::CheckCollisionPointRec(mousePos, ChatWindow) && rl::IsMouseButtonDown(rl::MOUSE_BUTTON_LEFT))
    {
        // Compute the new position for the ChatWindow
        float newWindowX = mousePos.x - LeftClickPos.x;
        float newWindowY = mousePos.y - LeftClickPos.y;

        // Ensure that the user cannot drag menu out of screen area and update positions of window and buttons
        if (!(newWindowX <= 0.0f || newWindowX + ChatWindow.width >= sst::baseX))
        {
            ChatWindow.x = newWindowX;
        }
        if (!(newWindowY <= 0.0f || newWindowY + ChatWindow.height >= sst::baseY))
        {
            ChatWindow.y = newWindowY;
        }
    }
}

float ChatModule::GetCenterX() const
{
    return CHATWINDOW_WIDTH / 2;
}

float ChatModule::GetCenterY() const
{
    return CHATWINDOW_HEIGHT / 2;
}

const rl::Rectangle ChatModule::CenterRect(int width, int height) const
{
    return (rl::Rectangle){
        GetCenterX() - (width / 2),
        GetCenterY() - (height / 2),
        (float)width,
        (float)height};
}

void ChatModule::DrawTextAboveRect(const ChatModule_TextInfo &info, int padding) const
{
    rl::Rectangle tempRec = {
        info.textBox.location.x + CHATWINDOW_OFFSET_X,
        info.textBox.location.y + CHATWINDOW_OFFSET_Y,
        info.textBox.location.width,
        info.textBox.location.height};
    if (!info.hasTextBox)
    {
    rl::DrawText(
        info.text.c_str(),
        (info.textBox.location.x + info.textBox.location.width / 2) - rl::MeasureText(info.text.c_str(), info.font) / 2 + CHATWINDOW_OFFSET_X,
        info.textBox.location.y - info.font - padding + CHATWINDOW_OFFSET_Y,
        info.font,
        info.isMouseHovering ? info.selectColor : info.nonSelectColor);
        rl::DrawRectangleLinesEx(tempRec, info.textBox.thickness, info.textBox.color);
    } else {
        rl::DrawText(
        info.text.c_str(),
        (info.textBox.location.x + info.textBox.location.width / 2) - rl::MeasureText(info.text.c_str(), info.font) / 2 + CHATWINDOW_OFFSET_X,
        info.textBox.location.y - info.font - padding + CHATWINDOW_OFFSET_Y,
        info.font,
        info.nonSelectColor);
        rl::DrawRectangleLinesEx(tempRec, info.textBox.thickness, info.textBox.isEditing ? info.selectColor : (info.isMouseHovering ? info.selectColor : info.nonSelectColor));
    }
}
void ChatModule::DrawTextBelowRect(const ChatModule_TextInfo &info, int padding) const
{
    rl::DrawText(
        info.text.c_str(),
        (info.textBox.location.x + info.textBox.location.width / 2) - rl::MeasureText(info.text.c_str(), info.font) / 2,
        info.textBox.location.y + info.textBox.location.height + padding,
        info.font,
        info.isMouseHovering ? info.selectColor : info.nonSelectColor);
    rl::DrawRectangleLinesEx(info.textBox.location, info.textBox.thickness, info.textBox.color);
}
void ChatModule::DrawTextLeftOfRect(const ChatModule_TextInfo &info, int padding) const
{
    rl::DrawText(
        info.text.c_str(),
        info.textBox.location.x - rl::MeasureText(info.text.c_str(), info.font) - padding,
        (info.textBox.location.y + info.textBox.location.height / 2) - info.font / 2,
        info.font,
        info.isMouseHovering ? info.selectColor : info.nonSelectColor);
    rl::DrawRectangleLinesEx(info.textBox.location, info.textBox.thickness, info.textBox.color);
}
void ChatModule::DrawTextRightOfRect(const ChatModule_TextInfo &info, int padding) const
{
    rl::DrawText(
        info.text.c_str(),
        info.textBox.location.x + info.textBox.location.width + padding,
        (info.textBox.location.y + info.textBox.location.height / 2) - info.font / 2,
        info.font,
        info.isMouseHovering ? info.selectColor : info.nonSelectColor);
    rl::DrawRectangleLinesEx(info.textBox.location, info.textBox.thickness, info.textBox.color);
}

void ChatModule::DrawTextOnPoint(const ChatModule_TextInfo &info) const
{
    rl::DrawText(
        info.text.c_str(),
        info.textLocationPoint.x - rl::MeasureText(info.text.c_str(), info.font) / 2 + CHATWINDOW_OFFSET_X,
        info.textLocationPoint.y - (info.font / 2) + CHATWINDOW_OFFSET_Y,
        info.font,
        info.isMouseHovering ? info.selectColor : info.nonSelectColor);
}

void ChatModule::DrawTextLeftOfPoint(const ChatModule_TextInfo &info) const
{
    rl::DrawText(
        info.text.c_str(),
        info.textLocationPoint.x - rl::MeasureText(info.text.c_str(), info.font) + CHATWINDOW_OFFSET_X,
        info.textLocationPoint.y - (info.font / 2) + CHATWINDOW_OFFSET_Y,
        info.font,
        info.isMouseHovering ? info.selectColor : info.nonSelectColor);
}

void ChatModule::InitLoadingImage()
{
    // Load and rescale loading image to fit on base resolution
    std::string filepath = "./art/loadingCircle.png";
    rl::Image loadingImage = rl::LoadImage(filepath.c_str());
    T_loading.texture = rl::LoadTextureFromImage(loadingImage);
    T_loading.sourceRec = (rl::Rectangle){0, 0, 800, 800}; // Original dimensions of the texture
    T_loading.textureCenter = (rl::Vector2){0.0f + 50, 0.0f + 50};
    T_loading.currentAngle = 0.0f;
    T_loading.animationStages = 12;
    T_loading.framesCounted = 0;
    T_loading.framesUntilUpdate = 5;
    T_loading.currentAnimationStage = 0;
}

void ChatModule::ButtonInfoInit()
{
    buttonInfo.font = 19;
    buttonInfo.nonSelectColor = rl::WHITE;
    buttonInfo.selectColor = rl::RED;
    buttonInfo.textBox.thickness = 1.0f;
    buttonInfo.textBox.color = rl::WHITE;
    buttonInfo.hasTextBox = false;
    buttonInfo.isMouseHovering = false;
    buttonInfo.textBox.type = None;
    buttonInfo.textBox.isEditing = false;
    buttonInfo.locationOrientation = Center;
}

void ChatModule::CheckButtonCollision()
{
    rl::Vector2 mousePos = rl::GetMousePosition();
    rl::Rectangle buttonRect;
    for (auto& button : buttons)
    {
        if (button.second.hasTextBox)
        {   
            if (rl::CheckCollisionPointRec(mousePos,
            (rl::Rectangle){button.second.textBox.location.x + CHATWINDOW_OFFSET_X,
             button.second.textBox.location.y + CHATWINDOW_OFFSET_Y,
            button.second.textBox.location.width, 
             button.second.textBox.location.height}))
            {
                button.second.isMouseHovering = true;
            } else {
                button.second.isMouseHovering = false;
            }
        } else {
            //Check where text is being drawn based off button to get correct hitbox detection
            switch(button.second.locationOrientation)
            {
                case Center:
                    GetRectFromPoint_Center(button.second, buttonRect);
                    break;
                case Top:
                    GetRectFromPoint_Top(button.second, buttonRect);
                    break;
                case Left:
                    GetRectFromPoint_Left(button.second, buttonRect);
                    break;
                case Right:
                    GetRectFromPoint_Right(button.second, buttonRect);
                    break;
                case Bottom:
                    GetRectFromPoint_Bottom(button.second, buttonRect);
                    break;
            }
            rl::CheckCollisionPointRec(mousePos, buttonRect) ? button.second.isMouseHovering = true : button.second.isMouseHovering = false;
        }
    }
}

void ChatModule::ModifyTextBox()
{
    isTyping ? rl::SetMouseCursor(rl::MOUSE_CURSOR_IBEAM) : rl::SetMouseCursor(rl::MOUSE_CURSOR_DEFAULT);
    for (auto& button : buttons)
    {
        if (!button.second.hasTextBox)
            continue;
        if (!button.second.textBox.isEditing)
            continue;
        int key = rl::GetCharPressed();
        switch(button.second.textBox.type)
        {
            case None:
                continue;
                break;
            case Username:
                while (key > 0)
                {
                    if ((key >= 32) && (key <= 125) && usernameLength < MAX_USERNAME_LENGTH)
                    {
                        username[usernameLength] = (char)key;
                        username[usernameLength + 1] = '\0';
                        usernameLength++;
                    }
                    key = rl::GetCharPressed();
                }
                if (rl::IsKeyPressed(rl::KEY_BACKSPACE))
                {
                    usernameLength--;
                    if (usernameLength < 0) usernameLength = 0;
                    username[usernameLength] = '\0';
                }
                break;
            case Port:
                while (key > 0)
                {
                    if ((((key >= 48) && (key <= 57)) || key == 46) && portLength < MAX_PORTNUMBER_LENGTH)
                    {
                        port[portLength] = (char)key;
                        port[portLength + 1] = '\0';
                        portLength++;
                    }
                    key = rl::GetCharPressed();
                }
                if (rl::IsKeyPressed(rl::KEY_BACKSPACE))
                {
                    portLength--;
                    if (portLength < 0) portLength = 0;
                    port[portLength] = '\0';
                }
                break;
            case Ip:

                break;
            case Message:

                break;
        }
        
    }
}

void ChatModule::DrawTextBox(const ChatModule_TextInfo &info, ChatModule_EditingTextBox textboxType, bool isCentered) const
{
    switch(textboxType)
    {
        case None:
            break;
        case Username:
            if (isCentered)
            {
                rl::DrawText(
                    username,
                    info.textBox.location.x + CHATWINDOW_OFFSET_X + info.textBox.location.width/2 - rl::MeasureText(username, info.font)/2,
                    info.textBox.location.y + (float)(info.textBox.location.height - info.font)/2 + CHATWINDOW_OFFSET_Y,
                    info.font, info.nonSelectColor);
            } else {
                rl::DrawText(
                    username,
                    info.textBox.location.x + CHATWINDOW_OFFSET_X,
                    info.textBox.location.y + (float)(info.textBox.location.height - info.font)/2 + CHATWINDOW_OFFSET_Y,
                    info.font, info.nonSelectColor);
            }
            break;
        case Port:
            if (isCentered)
            {
                rl::DrawText(
                    port,
                    info.textBox.location.x + CHATWINDOW_OFFSET_X + info.textBox.location.width/2 - rl::MeasureText(port, info.font)/2,
                    info.textBox.location.y + (float)(info.textBox.location.height - info.font)/2 + CHATWINDOW_OFFSET_Y,
                    info.font, info.nonSelectColor);
            } else {
                rl::DrawText(
                    port,
                    info.textBox.location.x + CHATWINDOW_OFFSET_X,
                    info.textBox.location.y + (float)(info.textBox.location.height - info.font)/2 + CHATWINDOW_OFFSET_Y,
                    info.font, info.nonSelectColor);
            }
            break;
        case Ip:
            break;
        case Message:
            break;

    }
}

//Assuming DrawTextOnPoint is used
void ChatModule::GetRectFromPoint_Center(const ChatModule_TextInfo &info, rl::Rectangle& rec)
{
    rec.x = info.textLocationPoint.x - rl::MeasureText(info.text.c_str(), info.font) / 2 + CHATWINDOW_OFFSET_X;
    rec.y = info.textLocationPoint.y - info.font / 2 + CHATWINDOW_OFFSET_Y;
    rec.width = (float)rl::MeasureText(info.text.c_str(), info.font);
    rec.height = (float)info.font;
}

void ChatModule::GetRectFromPoint_Top(const ChatModule_TextInfo &info, rl::Rectangle& rec)
{
    
}

void ChatModule::GetRectFromPoint_Left(const ChatModule_TextInfo &info, rl::Rectangle& rec)
{
    rec.x = info.textLocationPoint.x - rl::MeasureText(info.text.c_str(), info.font) + CHATWINDOW_OFFSET_X;
    rec.y = info.textLocationPoint.y - info.font / 2 + CHATWINDOW_OFFSET_Y;
    rec.width = (float)rl::MeasureText(info.text.c_str(), info.font);
    rec.height = (float)info.font;
}

void ChatModule::GetRectFromPoint_Right(const ChatModule_TextInfo &info, rl::Rectangle& rec)
{

}

void ChatModule::GetRectFromPoint_Bottom(const ChatModule_TextInfo &info, rl::Rectangle& rec)
{

}