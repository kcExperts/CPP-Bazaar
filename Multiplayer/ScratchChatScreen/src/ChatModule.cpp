#include "ChatModule.h"

ChatModule::ChatModule()
{
    ChatWindow = rl::Rectangle{sst::baseX/2, sst::baseY/2, CHATWINDOW_WIDTH, CHATWINDOW_HEIGHT};
    WindowColor = rl::BLACK;
    currentMenu = Init;
    InitLoadingImage();
}

ChatModule::~ChatModule(){};

void ChatModule::Draw() const
{
    rl::DrawRectangleRec(ChatWindow, WindowColor);
    switch (currentMenu)
    {
        case 0: 
            DrawInit();
            break;
        case 1:

            break;
        case 2:

            break;
        
        case 3:

            break;
        case 4:

            break;
        case 5:
            DrawLoading();
            break;
    }

}

void ChatModule::DrawLoading() const
{
    //What the fuck is going on with the line below
    rl::Rectangle loadingDest = {GetCenterX() + T_loading.textureCenter.x - 100/2, GetCenterY() + T_loading.textureCenter.y - 100/2, 100, 100};
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

void ChatModule::DrawInit() const
{
    rl::Rectangle textPosReference = {GetCenterX(), GetCenterY() - 50, 0, 0};
    ChatModule_TextInfo t;
    t.text = "Welcome to the Chat Module!";
    t.font = 19;
    t.color = rl::WHITE;
    DrawTextAboveRect(t, textPosReference, 0);

    textPosReference.y = GetCenterY();
    t.text = "Click anywhere inside to begin";
    DrawTextAboveRect(t, textPosReference, 0);
}

void ChatModule::UpdateInit()
{
    if (rl::IsMouseButtonPressed(rl::MOUSE_BUTTON_LEFT) && rl::CheckCollisionPointRec(rl::GetMousePosition(), ChatWindow))
        currentMenu = Loading;
}

void ChatModule::UpdateState()
{
    Drag();
    switch (currentMenu)
    {
        case 0: 
            UpdateInit();
            break;
        case 1:

            break;
        case 2:

            break;
        
        case 3:

            break;
        case 4:

            break;
        case 5:
            UpdateLoading();
            break;
    }
}

void ChatModule::Drag()
{
    rl::Vector2 mousePos = rl::GetMousePosition();

    //Store the current position of the mouse during a left click to use as the reference point for the starting drag position
    if (rl::IsMouseButtonPressed(rl::MOUSE_BUTTON_LEFT))
    {
        if (rl::CheckCollisionPointRec(mousePos, ChatWindow))
            LeftClickPos = (rl::Vector2){mousePos.x - ChatWindow.x, mousePos.y - ChatWindow.y};
    }
    
    //Ensure that the user is dragging the chatwindow
    if (rl::CheckCollisionPointRec(mousePos, ChatWindow) && rl::IsMouseButtonDown(rl::MOUSE_BUTTON_LEFT))
    {
        //Compute the new position for the ChatWindow
        float newWindowX = mousePos.x - LeftClickPos.x;
        float newWindowY = mousePos.y - LeftClickPos.y;

        //Ensure that the user cannot drag menu out of screen area
        if (!(newWindowX <= 0.0f || newWindowX + ChatWindow.width >= sst::baseX))
            ChatWindow.x = newWindowX;
        if (!(newWindowY <= 0.0f || newWindowY + ChatWindow.height >= sst::baseY))
            ChatWindow.y = newWindowY;
    }
}



float ChatModule::GetCenterX() const
{
    return ChatWindow.x + CHATWINDOW_WIDTH/2;
}

float ChatModule::GetCenterY() const
{
    return ChatWindow.y + CHATWINDOW_HEIGHT/2;
}

const rl::Rectangle ChatModule::CenterRect(const rl::Rectangle& rect) const
{
    return (rl::Rectangle)
    {
        GetCenterX() - (rect.width/2),
        GetCenterY() - (rect.height/2),
        rect.width,
        rect.height
    };
}

void ChatModule::DrawTextAboveRect(const ChatModule_TextInfo& info, const rl::Rectangle& rect, int padding) const
{
    rl::DrawText(
        info.text.c_str(),
        (rect.x + rect.width/2) - rl::MeasureText(info.text.c_str(), info.font)/2,
        rect.y - info.font - padding,
        info.font,
        info.color
    );
}
void ChatModule::DrawTextBelowRect(const ChatModule_TextInfo& info, const rl::Rectangle& rect, int padding) const
{
    rl::DrawText(
        info.text.c_str(),
        (rect.x + rect.width/2) - rl::MeasureText(info.text.c_str(), info.font)/2,
        rect.y + rect.height + padding,
        info.font,
        info.color
    );
}
void ChatModule::DrawTextLeftOfRect(const ChatModule_TextInfo& info, const rl::Rectangle& rect, int padding) const
{
    rl::DrawText(
        info.text.c_str(),
        rect.x - rl::MeasureText(info.text.c_str(), info.font) - padding,
        (rect.y + rect.height/2) - info.font/2,
        info.font,
        info.color
    );
}
void ChatModule::DrawTextRightOfRect(const ChatModule_TextInfo& info, const rl::Rectangle& rect, int padding) const
{
    rl::DrawText(
        info.text.c_str(),
        rect.x + rect.width + padding,
        (rect.y + rect.height/2) - info.font/2,
        info.font,
        info.color
    );
}

void ChatModule::InitLoadingImage()
{
    //Load and rescale loading image to fit on base resolution
    std::string filepath = "./art/loadingCircle.png";
    rl::Image loadingImage = rl::LoadImage(filepath.c_str());
    T_loading.texture = rl::LoadTextureFromImage(loadingImage);
    T_loading.sourceRec = (rl::Rectangle){0, 0, 800, 800}; //Original dimensions of the texture
    T_loading.textureCenter = (rl::Vector2){0.0f + 50, 0.0f + 50};
    T_loading.currentAngle = 0.0f;
    T_loading.animationStages = 12;
    T_loading.framesCounted = 0;
    T_loading.framesUntilUpdate = 5;
    T_loading.currentAnimationStage = 0;
}

