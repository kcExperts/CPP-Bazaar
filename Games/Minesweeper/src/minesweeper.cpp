#include "minesweeper.h"

Minesweeper::Minesweeper(int fps)
{
    program_fps = fps;
    isDead = false;
    didWin = false;
    debug_viewer = false;
    isMenuInitialized = false;
    current_Menu = MM_Init;
    usernameLength = 0;
    portLength = 0;
    ipLength = 0;
    username[0] = '\0';
    port[0] = '\0';
    ip[0] = '\0';
    isTyping = false;
    difficulty = 1;
    tileSelected = -1;
    mouseInGame = false;
    isServer = false;
    std::srand((unsigned) time(NULL));
}

Minesweeper::~Minesweeper(){}

void Minesweeper::Draw()
{
    switch (current_Menu)
    {
        case MM_Init: 
            if (!isMenuInitialized) Initialize_Init();
            Draw_Init();
            break;
        case MM_Host:
            if (!isMenuInitialized) Initialize_Host();
            Draw_Host();
            break;
        case MM_Join:
            if (!isMenuInitialized) Initialize_Join();
            Draw_Join();
            break;
        case MM_CreateServer:
            if (!isMenuInitialized) Initialize_CreateServer();
            Draw_CreateServer();
            break;
        case MM_CreateClient:
            if (!isMenuInitialized) Initialize_CreateClient();
            Draw_CreateClient();
            break;
        case MM_ServerLobby:
            if (!isMenuInitialized) Initialize_ServerLobby();
            Draw_ServerLobby();
            break;
        case MM_ClientLobby:
            if (!isMenuInitialized) Initialize_ClientLobby();
            Draw_ClientLobby();
            break;
        case MM_Game:
            if (!isMenuInitialized) Initialize_Game();
            Draw_Game();
            break;
        case MM_AfterScreen:
            if (!isMenuInitialized) Initialize_AfterScreen();
            Draw_AfterScreen();
            break;
    }
}

void Minesweeper::UpdateState()
{
    switch (current_Menu)
    {
        case MM_Init: 
            Update_Init();
            break;
        case MM_Host:
            Update_Host();
            break;
        case MM_Join:
            Update_Join();
            break;
        case MM_CreateServer:
            Update_CreateServer();
            break;
        case MM_CreateClient:
            Update_CreateClient();
            break;
        case MM_ServerLobby:
            Update_ServerLobby();
            break;
        case MM_ClientLobby:
            Update_ClientLobby();
            break;
        case MM_Game:
            Update_Game();
            break;
        case MM_AfterScreen:
            Update_AfterScreen();
            break;
    }
}

void Minesweeper::Initialize_ClientLobby()
{
    BasicInitialize();
    button_Info.textLocationPoint = {MINESWEEPER_CENTER_X - 200, MINESWEEPER_CENTER_Y - 200};
    button_Info.text = "Disconnect";
    buttons["Leave"] = button_Info;  
    button_Info.textLocationPoint = {MINESWEEPER_CENTER_X - 200, MINESWEEPER_CENTER_Y};
    button_Info.nonSelectColor = rl::BLACK;
    button_Info.selectColor = button_Info.nonSelectColor;
    button_Info.text = "Waiting for server to begin";
    buttons["Desc"] = button_Info;
}

void Minesweeper::Draw_ClientLobby()
{
    chatModule->Draw();
    DrawTextOnPoint(buttons.at("Desc"));
    DrawTextOnPoint(buttons.at("Leave"));
}

void Minesweeper::Update_ClientLobby()
{
    CheckButtonCollision();
    if (rl::IsMouseButtonPressed(rl::MOUSE_BUTTON_LEFT))
    {
        if (buttons.at("Leave").isMouseHovering && !isTyping)
        {
            current_Menu = MM_Join;
            isMenuInitialized = false;
            chatModule = nullptr;
        }
    }
}

void Minesweeper::Initialize_AfterScreen()
{
    BasicInitialize();
    button_Info.textLocationPoint = {MINESWEEPER_CENTER_X - 200, MINESWEEPER_CENTER_Y + 100};
    button_Info.text = "Leave";
    buttons["Leave"] = button_Info;    
    if (isServer)
    {
        button_Info.text = "Play Again";
        button_Info.textLocationPoint.y -= 200;
        buttons["isServer"] = button_Info;
    } else {
        button_Info.text = "Waiting for Server to Continue";
        button_Info.textLocationPoint.y -= 200;
        button_Info.nonSelectColor = rl::BLACK;
        button_Info.selectColor = button_Info.nonSelectColor;
        buttons["isClient"] = button_Info;
    }
    if (didWin) button_Info.text = "You have won!";
    if (isDead) button_Info.text = "You have died!";
    button_Info.nonSelectColor = rl::BLACK;
    button_Info.selectColor = button_Info.nonSelectColor;
    button_Info.textLocationPoint = {MINESWEEPER_CENTER_X - 200, MINESWEEPER_CENTER_Y - 300};
    buttons["Desc"] = button_Info;
    Delete_Map();
}

void Minesweeper::Draw_AfterScreen()
{
    chatModule->Draw();
    DrawTextOnPoint(buttons.at("Desc"));
    DrawTextOnPoint(buttons.at("Leave"));
    if (isServer) {DrawTextOnPoint(buttons.at("isServer"));}
    else {DrawTextOnPoint(buttons.at("isClient"));}
}

void Minesweeper::Update_AfterScreen()
{
    CheckButtonCollision();
    if (rl::IsMouseButtonPressed(rl::MOUSE_BUTTON_LEFT))
    {
        if (isServer)
        {
            if (buttons.at("isServer").isMouseHovering && !isTyping)
                {
                    current_Menu = MM_ServerLobby;
                    isMenuInitialized = false;
                }
        }
        if (buttons.at("Leave").isMouseHovering && !isTyping)
        {
            if (isServer)
            {
                current_Menu = MM_Host;
                isMenuInitialized = false;
                chatModule = nullptr;
            } else {
                current_Menu = MM_Join;
                isMenuInitialized = false;
                chatModule = nullptr;               
            }
        }
    }
}

void Minesweeper::Initialize_Game()
{
    didWin = false;
    isDead = false;
    BasicInitialize();
    Update_Difficulty();
    Generate_Map();
}

void Minesweeper::Draw_Game()
{
    rl::DrawText(rl::TextFormat("Total Mines: %i", total_mines), 
    sst::cx(1000 - rl::MeasureText(rl::TextFormat("Total Mines: %i", total_mines), MINESWEEPER_STANDARD_FONT)/2),
    sst::cy(sst::baseY - 100),
    sst::cx(MINESWEEPER_STANDARD_FONT), rl::BLUE);
    chatModule->Draw();
    Draw_Map();
}

void Minesweeper::Update_Game()
{
    chatModule->UpdateState();
    rl::Vector2 mousePos = rl::GetMousePosition();
    rl::Rectangle gameRec = {0, 0, sst::cyf(sst::baseY), sst::cyf(sst::baseY)};
    mouseInGame = rl::CheckCollisionPointRec(mousePos, gameRec);
    if (rl::IsMouseButtonPressed(rl::MOUSE_BUTTON_LEFT))
    {
        if (mouseInGame && !map[tileSelected].flagged)
        {
            Reveal_Tile(tileSelected);
            if (map[tileSelected].is_mine)
            {
                isDead = true;
                didWin = false;
                //Send to client
                current_Menu = MM_AfterScreen;
                isMenuInitialized = false;
                return;
            }
        }
    }
    if (rl::IsMouseButtonPressed(rl::MOUSE_BUTTON_RIGHT))
    {
        if (mouseInGame && !map[tileSelected].revealed)
        {
            map[tileSelected].flagged = !map[tileSelected].flagged;
        }
    }
    if (didWin)
    {
        current_Menu = MM_AfterScreen;
        isMenuInitialized = false;
    }
    
}

void Minesweeper::Initialize_ServerLobby()
{
    BasicInitialize();
    button_Info.text = "Easy";
    button_Info.textLocationPoint = {MINESWEEPER_CENTER_X - 200, MINESWEEPER_CENTER_Y - 200};
    buttons["Easy"] = button_Info;
    button_Info.text = "Medium";
    button_Info.textLocationPoint.y += 100;
    buttons["Medium"] = button_Info;
    button_Info.text = "Hard";
    button_Info.textLocationPoint.y += 100;
    buttons["Hard"] = button_Info;
    button_Info.text = "Very Hard";
    button_Info.textLocationPoint.y += 100;
    buttons["Very Hard"] = button_Info;
    button_Info.text = "Extremely Hard";
    button_Info.textLocationPoint.y += 100;
    buttons["Extremely Hard"] = button_Info;
    button_Info.text = "Start";
    button_Info.textLocationPoint.y += 100;
    buttons["Start"] = button_Info;
    button_Info.text = "Choose Map Difficulty";
    button_Info.nonSelectColor = rl::BLACK;
    button_Info.selectColor = button_Info.nonSelectColor;
    button_Info.textLocationPoint = {MINESWEEPER_CENTER_X - 200, MINESWEEPER_CENTER_Y - 300};
    buttons["Desc"] = button_Info;
}

void Minesweeper::Draw_ServerLobby()
{
    chatModule->Draw();
    DrawTextOnPoint(buttons.at("Desc"));
    DrawTextOnPoint(buttons.at("Easy"));
    DrawTextOnPoint(buttons.at("Medium"));
    DrawTextOnPoint(buttons.at("Hard"));
    DrawTextOnPoint(buttons.at("Very Hard"));
    DrawTextOnPoint(buttons.at("Extremely Hard"));
    DrawTextOnPoint(buttons.at("Start"));
}

void Minesweeper::Update_ServerLobby()
{
    chatModule->UpdateState();
    CheckButtonCollision();
    if (rl::IsMouseButtonPressed(rl::MOUSE_BUTTON_LEFT))
    {
        if (buttons.at("Easy").isMouseHovering && !isTyping)
        {
            difficulty = 1;
        }
        if (buttons.at("Medium").isMouseHovering && !isTyping)
        {
            difficulty = 2;
        }
        if (buttons.at("Hard").isMouseHovering && !isTyping)
        {
            difficulty = 3;
        }
        if (buttons.at("Very Hard").isMouseHovering && !isTyping)
        {
            difficulty = 4;
        }
        if (buttons.at("Extremely Hard").isMouseHovering && !isTyping)
        {
            difficulty = 5;
        }
        if (buttons.at("Start").isMouseHovering && !isTyping)
        {
            isMenuInitialized = false;
            current_Menu = MM_Game;
        }
    }
    for (auto& button : buttons)
    {
        if (button.first == "Desc") continue;
        button.second.nonSelectColor = rl::BLUE;
    }
    switch(difficulty)
    {
        case 1:
            buttons.at("Easy").nonSelectColor = rl::RED;
            break;
        case 2:
            buttons.at("Medium").nonSelectColor = rl::RED;
            break;
        case 3:
            buttons.at("Hard").nonSelectColor = rl::RED;
            break;
        case 4:
            buttons.at("Very Hard").nonSelectColor = rl::RED;
            break;
        case 5:
            buttons.at("Extremely Hard").nonSelectColor = rl::RED;
            break;
    }
}

void Minesweeper::Initialize_CreateClient()
{
    BasicInitialize();
    chatModule = std::make_unique<ChatModule>(program_fps, false, ip, port, username);
    button_Info.textLocationPoint = {MINESWEEPER_CENTER_X - 200, MINESWEEPER_CENTER_Y};
    button_Info.text = "Attempting to Join Server";
    button_Info.nonSelectColor = rl::BLACK;
    button_Info.selectColor = button_Info.nonSelectColor;
    buttons["Waiting"] = button_Info;  
}

void Minesweeper::Draw_CreateClient()
{
    chatModule->Draw();
    DrawTextOnPoint(buttons.at("Waiting"));
}

void Minesweeper::Update_CreateClient()
{
    chatModule->UpdateState();
    int connectingStatus = chatModule->getDidConnectionWork();
    if (connectingStatus == -1) return;
    if (connectingStatus == 0)
    {
        chatModule = nullptr;
        isMenuInitialized = false;
        current_Menu = MM_Join;
        return;
    }
    if (connectingStatus == 1)
    {
        isMenuInitialized = false;
        current_Menu = MM_ClientLobby;
        return;
    }
}

void Minesweeper::Initialize_CreateServer()
{
    BasicInitialize();
    chatModule = std::make_unique<ChatModule>(program_fps, true, ip, port, username);
}

void Minesweeper::Draw_CreateServer()
{
    chatModule->Draw();
}

void Minesweeper::Update_CreateServer()
{
    chatModule->UpdateState();
    int connectingStatus = chatModule->getDidConnectionWork();
    if (connectingStatus == -1) return;
    if (connectingStatus == 0)
    {
        chatModule = nullptr;
        isMenuInitialized = false;
        current_Menu = MM_Host;
        return;
    }
    if (connectingStatus == 1)
    {
        isMenuInitialized = false;
        current_Menu = MM_ServerLobby;
        return;
    }
}

void Minesweeper::Initialize_Join()
{
    BasicInitialize();
    button_Info.hasTextBox = false;
    button_Info.textOrientation = MLO_Left;
    button_Info.text = "Back";
    button_Info.textLocationPoint = {MINESWEEPER_TEXT_RIGHT_X, MINESWEEPER_TEXT_TOP_Y};
    buttons["Back"] = button_Info;
    button_Info.text = "Join";
    button_Info.textLocationPoint.y = MINESWEEPER_TEXT_BOTTOM_Y;
    buttons["Join"] = button_Info;
    button_Info.textOrientation = MLO_Center;
    button_Info.hasTextBox = true;
    rl::Rectangle rec = CenterRect(MINESWEEPER_DEFAULT_TEXTBOX_WIDTH, MINESWEEPER_DEFAULT_TEXTBOX_HEIGHT);
    rec.y -= 200;
    button_Info.textBox.location = rec;
    button_Info.textBox.type = MTBT_USERNAME;
    button_Info.text = "Insert Username Below";
    buttons["Userbox"] = button_Info;
    rec = CenterRect(MINESWEEPER_DEFAULT_TEXTBOX_WIDTH - 100, MINESWEEPER_DEFAULT_TEXTBOX_HEIGHT);
    button_Info.textBox.location = rec;
    //button_Info.textBox.location.y += 100;
    button_Info.textBox.type = MTBT_PORT;
    button_Info.text = "Insert Host Port Number Below";
    buttons["Portbox"] = button_Info;
    rec = CenterRect(MINESWEEPER_DEFAULT_TEXTBOX_WIDTH, MINESWEEPER_DEFAULT_TEXTBOX_HEIGHT);
    button_Info.textBox.location = rec;
    button_Info.textBox.location.y += 200;
    button_Info.textBox.type = MTBT_IP;
    button_Info.text = "Insert Host IP Address Below";
    buttons["Ipbox"] = button_Info;
}

void Minesweeper::Draw_Join()
{
    DrawTextAboveRect(buttons.at("Userbox"), 10);
    DrawTextAboveRect(buttons.at("Portbox"), 10);
    DrawTextAboveRect(buttons.at("Ipbox"), 10);
    DrawTextLeftOfPoint(buttons.at("Back"));
    DrawTextLeftOfPoint(buttons.at("Join"));
}

void Minesweeper::Update_Join()
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
        isE = buttons.at("Ipbox").textBox.isEditing;
        if (buttons.at("Ipbox").isMouseHovering && ((!isTyping && !isE) || (isTyping && isE)))
        {
            buttons.at("Ipbox").textBox.isEditing = !buttons.at("Ipbox").textBox.isEditing;
            isTyping = !isTyping;
        }
        if (buttons.at("Back").isMouseHovering && !isTyping)
        {
            isMenuInitialized = false;
            current_Menu = MM_Init;
        }
        if (buttons.at("Join").isMouseHovering && !isTyping)
        {
            isMenuInitialized = false;
            current_Menu = MM_CreateClient;
        }
    }
}

void Minesweeper::Initialize_Host()
{
    BasicInitialize();
    button_Info.hasTextBox = false;
    button_Info.textOrientation = MLO_Left;
    button_Info.text = "Back";
    button_Info.textLocationPoint = {MINESWEEPER_TEXT_RIGHT_X, MINESWEEPER_TEXT_TOP_Y};
    buttons["Back"] = button_Info;
    button_Info.text = "Create";
    button_Info.textLocationPoint.y = MINESWEEPER_TEXT_BOTTOM_Y;
    buttons["Create"] = button_Info;
    button_Info.textOrientation = MLO_Center;
    button_Info.hasTextBox = true;
    rl::Rectangle rec = CenterRect(MINESWEEPER_DEFAULT_TEXTBOX_WIDTH, MINESWEEPER_DEFAULT_TEXTBOX_HEIGHT);
    rec.y -= 100;
    button_Info.textBox.location = rec;
    button_Info.textBox.type = MTBT_USERNAME;
    button_Info.text = "Insert Username Below";
    buttons["Userbox"] = button_Info;
    rec = CenterRect(MINESWEEPER_DEFAULT_TEXTBOX_WIDTH - 100, MINESWEEPER_DEFAULT_TEXTBOX_HEIGHT);
    button_Info.textBox.location = rec;
    button_Info.textBox.location.y += 100;
    button_Info.textBox.type = MTBT_PORT;
    button_Info.text = "Insert Port Number Below";
    buttons["Portbox"] = button_Info;
}

void Minesweeper::Draw_Host()
{
    DrawTextLeftOfPoint(buttons.at("Back"));
    DrawTextLeftOfPoint(buttons.at("Create"));
    DrawTextAboveRect(buttons.at("Userbox"), 10);
    DrawTextAboveRect(buttons.at("Portbox"), 10);
}

void Minesweeper::Update_Host()
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
            isMenuInitialized = false;
            current_Menu = MM_Init;
        }
        if (buttons.at("Create").isMouseHovering && !isTyping)
        {
            isMenuInitialized = false;
            current_Menu = MM_CreateServer;
        }
    }
}

void Minesweeper::Initialize_Init()
{
    isServer = false;
    BasicInitialize();
    button_Info.textLocationPoint = {MINESWEEPER_CENTER_X, MINESWEEPER_CENTER_Y};
    button_Info.text = "Host";
    buttons["Host"] = button_Info;
    button_Info.textLocationPoint.y += 150;
    button_Info.text = "Join";
    buttons["Join"] = button_Info;
    button_Info.textLocationPoint = {MINESWEEPER_CENTER_X, MINESWEEPER_CENTER_Y - 200};
    button_Info.text = "MINESWEEPER";
    button_Info.fontSize = 70;
    button_Info.nonSelectColor = rl::BLACK;
    button_Info.selectColor = button_Info.nonSelectColor;
    buttons["Minesweeper"] = button_Info;
}

void Minesweeper::Draw_Init()
{
    DrawTextOnPoint(buttons.at("Minesweeper"));
    DrawTextOnPoint(buttons.at("Host"));
    DrawTextOnPoint(buttons.at("Join"));
}

void Minesweeper::Update_Init()
{
    CheckButtonCollision();
    if (rl::IsMouseButtonPressed(rl::MOUSE_BUTTON_LEFT))
    {
        if (buttons.at("Host").isMouseHovering)
        {
            isMenuInitialized = false;
            isServer = true;
            current_Menu = MM_Host;
        }
        if (buttons.at("Join").isMouseHovering)
        {
            isMenuInitialized = false;
            isServer = false;
            current_Menu = MM_Join;
        }
    }
}

void Minesweeper::BasicInitialize()
{
    buttons.clear();
    isMenuInitialized = true; 
    button_Info.fontSize = MINESWEEPER_STANDARD_FONT;
    button_Info.nonSelectColor = rl::BLUE;
    button_Info.selectColor = rl::RED;
    button_Info.hasTextBox = false;
    button_Info.isMouseHovering = false;
    button_Info.textOrientation = MLO_Center;
    button_Info.textBox.thickness = 2.0f;
    button_Info.textBox.isEditing = false;
    button_Info.textBox.color = rl::BLACK;
    button_Info.textBox.isTextCentered = true;
}

void Minesweeper::DrawTextOnPoint(const Minesweeper_Text_Info &info) const
{
    rl::DrawText(
        info.text.c_str(),
        sst::cx(info.textLocationPoint.x - rl::MeasureText(info.text.c_str(), info.fontSize) / 2),
        sst::cy(info.textLocationPoint.y - (info.fontSize / 2)),
        sst::cx(info.fontSize),
        info.isMouseHovering ? info.selectColor : info.nonSelectColor);
}

void Minesweeper::CheckButtonCollision()
{
    rl::Vector2 mousePos = rl::GetMousePosition();
    rl::Rectangle buttonRect;
    for (auto& button : buttons)
    {
        if (button.second.hasTextBox)
        {
            if (rl::CheckCollisionPointRec(mousePos,
            (rl::Rectangle){sst::cxf(button.second.textBox.location.x),
            sst::cyf(button.second.textBox.location.y),
            sst::cxf(button.second.textBox.location.width), 
            sst::cyf(button.second.textBox.location.height)}))
            {
                button.second.isMouseHovering = true;
            } else {
                button.second.isMouseHovering = false;
            }
        } else {
            GetRectFromPoint(button.second, buttonRect);
            rl::CheckCollisionPointRec(mousePos, buttonRect) ? button.second.isMouseHovering = true : button.second.isMouseHovering = false;
        }
    }
}

void Minesweeper::GetRectFromPoint(const Minesweeper_Text_Info &info, rl::Rectangle &rec)
{
    switch(info.textOrientation)
    {
        case Center:
            rec.x = sst::cxf(info.textLocationPoint.x - rl::MeasureText(info.text.c_str(), info.fontSize) / 2);
            rec.y = sst::cyf(info.textLocationPoint.y - info.fontSize / 2);
            rec.width = sst::cxf((float)rl::MeasureText(info.text.c_str(), info.fontSize));
            rec.height = sst::cyf((float)info.fontSize);
            break;
        case Left:
            rec.x = sst::cxf(info.textLocationPoint.x - rl::MeasureText(info.text.c_str(), info.fontSize));
            rec.y = sst::cyf(info.textLocationPoint.y - info.fontSize / 2);
            rec.width = sst::cxf((float)rl::MeasureText(info.text.c_str(), info.fontSize));
            rec.height = sst::cyf((float)info.fontSize);
            break;
    }
}

void Minesweeper::DrawTextAboveRect(const Minesweeper_Text_Info &info, int padding) const
{
    rl::Rectangle tempRec = {
        sst::cxf(info.textBox.location.x),
        sst::cyf(info.textBox.location.y),
        sst::cxf(info.textBox.location.width),
        sst::cyf(info.textBox.location.height)};
    if (!info.hasTextBox)
    {
    rl::DrawText(
        info.text.c_str(),
        sst::cx((info.textBox.location.x + info.textBox.location.width / 2) - rl::MeasureText(info.text.c_str(), info.fontSize) / 2),
        sst::cy(info.textBox.location.y - info.fontSize - padding),
        sst::cx(info.fontSize),
        info.isMouseHovering ? info.selectColor : info.nonSelectColor);
        rl::DrawRectangleLinesEx(tempRec, sst::cxf(info.textBox.thickness), info.textBox.color);
    } else {
        rl::DrawText(
        info.text.c_str(),
        sst::cx((info.textBox.location.x + info.textBox.location.width / 2) - rl::MeasureText(info.text.c_str(), info.fontSize) / 2),
        sst::cy(info.textBox.location.y - info.fontSize - padding),
        sst::cx(info.fontSize),
        info.nonSelectColor);
        rl::DrawRectangleLinesEx(tempRec, sst::cxf(info.textBox.thickness), info.textBox.isEditing ? info.selectColor : (info.isMouseHovering ? info.selectColor : info.nonSelectColor));
        switch (info.textBox.type)
        {
            case MTBT_NONE:
                break;
            case MTBT_IP:
                if (info.textBox.isTextCentered)
                {
                    rl::DrawText(
                        ip,
                        sst::cx(info.textBox.location.x + info.textBox.location.width/2 - rl::MeasureText(ip, info.fontSize)/2),
                        sst::cy(info.textBox.location.y + (float)(info.textBox.location.height - info.fontSize)/2),
                        sst::cx(info.fontSize), info.nonSelectColor);
                } else {
                    rl::DrawText(
                        ip,
                        sst::cx(info.textBox.location.x + 5),
                        sst::cy(info.textBox.location.y + (float)(info.textBox.location.height - info.fontSize)/2),
                        sst::cx(info.fontSize), info.nonSelectColor);
                }
                break;
            case MTBT_PORT:
                if (info.textBox.isTextCentered)
                {
                    rl::DrawText(
                        port,
                        sst::cx(info.textBox.location.x + info.textBox.location.width/2 - rl::MeasureText(port, info.fontSize)/2),
                        sst::cy(info.textBox.location.y + (float)(info.textBox.location.height - info.fontSize)/2),
                        sst::cx(info.fontSize), info.nonSelectColor);
                } else {
                    rl::DrawText(
                        port,
                        sst::cx(info.textBox.location.x + 5),
                        sst::cy(info.textBox.location.y + (float)(info.textBox.location.height -info.fontSize)/2),
                        sst::cx(info.fontSize), info.nonSelectColor);
                }
                break;
            case MTBT_USERNAME:
                if (info.textBox.isTextCentered)
                {
                    rl::DrawText(
                        username,
                        sst::cx(info.textBox.location.x + info.textBox.location.width/2 - rl::MeasureText(username, info.fontSize)/2),
                        sst::cy(info.textBox.location.y + (float)(info.textBox.location.height - info.fontSize)/2),
                        sst::cx(info.fontSize), info.nonSelectColor);
                } else {
                    rl::DrawText(
                        username,
                        sst::cx(info.textBox.location.x + 5),
                        sst::cy(info.textBox.location.y + (float)(info.textBox.location.height - info.fontSize)/2),
                        sst::cx(info.fontSize), info.nonSelectColor);
                }
                break;
        }    
    }
}

void Minesweeper::ModifyTextBox()
{
    isTyping ? rl::SetMouseCursor(rl::MOUSE_CURSOR_IBEAM) : rl::SetMouseCursor(rl::MOUSE_CURSOR_DEFAULT);
    for (auto& button : buttons)
    {
        if (!button.second.hasTextBox) continue;
        if (!button.second.textBox.isEditing) continue;
        int key = rl::GetCharPressed();
        switch(button.second.textBox.type)
        {
            case MTBT_NONE:
                continue;
                break;
            case MTBT_IP:
                while (key > 0)
                {
                    if ((((key >= 48) && (key <= 57)) || key == 46) && ipLength < MAX_IP_LENGTH)
                    {
                        ip[ipLength] = (char)key;
                        ip[ipLength + 1] = '\0';
                        ipLength++;
                    }
                    key = rl::GetCharPressed();
                }
                if (rl::IsKeyPressed(rl::KEY_BACKSPACE))
                {
                    ipLength--;
                    if (ipLength < 0) ipLength = 0;
                    ip[ipLength] = '\0';
                }
                break;

            case MTBT_PORT:
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

            case MTBT_USERNAME:
                while (key > 0)
                {
                    if ((key >= 32) && (key <= 125) && (key != 92) && usernameLength < MAX_USERNAME_LEN)
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
        }
    }
}

void Minesweeper::DrawTextLeftOfPoint(const Minesweeper_Text_Info &info) const
{
    rl::DrawText(
        info.text.c_str(),
        sst::cx(info.textLocationPoint.x - rl::MeasureText(info.text.c_str(), info.fontSize)),
        sst::cy(info.textLocationPoint.y - (info.fontSize / 2)),
        sst::cx(info.fontSize),
        info.isMouseHovering ? info.selectColor : info.nonSelectColor);
}

const rl::Rectangle Minesweeper::CenterRect(int width, int height) const
{
    return (rl::Rectangle){
        MINESWEEPER_CENTER_X - ((float)width / 2),
        MINESWEEPER_CENTER_Y - ((float)height / 2),
        (float)width,
        (float)height};
}

rl::CLITERAL(Color) Minesweeper::Generate_Color(int mine_number)
{
    switch (mine_number)
    {
        case 0:
            return rl::PURPLE;
            break;
        case 1:
            return rl::BLUE;
            break;
        case 2:
            return rl::DARKGREEN;
            break;
        case 3:
            return rl::RED;
            break;
        case 4:
            return rl::DARKBLUE;
            break;
        case 5:
            return rl::BLACK;
            break;
        case 6:
            return rl::ORANGE;
            break;
        case 7:
            return rl::MAROON;
            break;
        case 8:
            return rl::DARKGRAY;
            break;
    }
    return rl::WHITE;
}


void Minesweeper::Update_Difficulty()
{
    map_size = 8 + (3 * difficulty); 
    total_mines = 8 + (10 * (difficulty - 1)); // 8 + (10 * difficulty)
    switch(difficulty)
    {
        case 1:
            mineNumFontSize = MINESWEEPER_STANDARD_FONT;
            break;
        case 2:
            mineNumFontSize = MINESWEEPER_STANDARD_FONT - 10;
            break;
        case 3:
            mineNumFontSize = MINESWEEPER_STANDARD_FONT - 15;
            break;
        case 4:
            mineNumFontSize = MINESWEEPER_STANDARD_FONT - 20;
            break;
        case 5:
            mineNumFontSize = MINESWEEPER_STANDARD_FONT - 25;
            break; 
    }
}

void Minesweeper::Generate_Map()
{
    //Initialize the map and the tile locations
    int random;
    int count = 0;
    float pos_x = 0;
    float pos_y = 0;
    float squareLength = (float)sst::baseY/(float)map_size;
    for (int i = 0; i < (map_size * map_size); i++)
    {
        map.push_back({false, 0, (rl::Rectangle){0, 0, 0, 0}, false, false});
        map[i].rec.x = pos_x;
        map[i].rec.y = pos_y;
        map[i].rec.height = squareLength;
        map[i].rec.width = squareLength;
        //Update rows by incrementing x then increment y and reset x
        pos_x += squareLength;
        count++;
        if (count > map_size - 1)
        {
            pos_x = 0;
            pos_y += squareLength;
            count = 0;
        }
    }
    count = 0;
    //Generate the mines
    while (count < total_mines)
    {
        random = rand() % (map.size() - 1);
        if (map[random].is_mine == false)
        {
            map[random].is_mine = true;
            map[random].mine_num = -1;
            count++;
        }
    }
    //Generate the number associated to a non-mine tile
    for (int i = 0; i < (map.size()); i++)
    {
        if (map[i].is_mine == false)
        {
            //This if checks if the cell is on the left-most row or the right-side row or it is neither
            if (i % map_size == 0)
            {
                if (i - map_size >= 0)
                {
                    if (map[i - map_size].is_mine == true)
                        map[i].mine_num++;
                    if (map[i - map_size + 1].is_mine == true)
                        map[i].mine_num++;
                }
                if (i + map_size <= (map_size * map_size) - 1) //essentially total tiles indexed starting at 0
                {
                    if (map[i + map_size].is_mine == true)
                        map[i].mine_num++;
                    if (map[i + map_size + 1].is_mine == true)
                        map[i].mine_num++;
                }
                if (map[i + 1].is_mine == true)
                    map[i].mine_num++;
            } else if (i %  map_size == map_size - 1) {
                if (i - map_size >= 0)
                {
                    if (map[i - map_size].is_mine == true)
                        map[i].mine_num++;
                    if (map[i - map_size - 1].is_mine == true)
                        map[i].mine_num++;
                }
                if (i + map_size <= (map_size * map_size) - 1)
                {
                    if (map[i + map_size].is_mine == true)
                        map[i].mine_num++;
                    if (map[i + map_size - 1].is_mine == true)
                        map[i].mine_num++;
                }
                if (map[i - 1].is_mine == true)
                    map[i].mine_num++;
            } else {
                if (i - map_size >= 0)
                {
                    if (map[i - map_size].is_mine == true)
                        map[i].mine_num++;
                    if (map[i - map_size - 1].is_mine == true)
                        map[i].mine_num++;
                    if (map[i - map_size + 1].is_mine == true)
                        map[i].mine_num++;
                }
                if (i + map_size <= (map_size * map_size) - 1)
                {
                    if (map[i + map_size].is_mine == true)
                        map[i].mine_num++;
                    if (map[i + map_size - 1].is_mine == true)
                        map[i].mine_num++;
                    if (map[i + map_size + 1].is_mine == true)
                        map[i].mine_num++;
                }
                if (map[i - 1].is_mine == true)
                    map[i].mine_num++;
                if (map[i + 1].is_mine == true)
                    map[i].mine_num++;
            }
        }
    }
}

void Minesweeper::Draw_Map()
{
    CLITERAL(rl::Color) color;
    int cor_flag = 0;
    int map_reveal = total_mines;
    rl::Vector2 mousePos = rl::GetMousePosition();

    for (int i = 0; i < map_size * map_size; i++)
    {
        rl::Rectangle tempRec = {sst::cxf(map[i].rec.x), sst::cyf(map[i].rec.y), sst::cxf(map[i].rec.width), sst::cyf(map[i].rec.height)};
        rl::Vector2 centerOfTempRec = {tempRec.x + tempRec.width/2, tempRec.y + tempRec.height/2};
        bool onTile = rl::CheckCollisionPointRec(mousePos, tempRec);
        if (mouseInGame)
        {
            if (onTile) tileSelected = i;
        } else {
            tileSelected = -1;
        }
        rl::DrawRectangleLinesEx(tempRec, sst::cxf(1.0f), onTile ? rl::GREEN : rl::BLACK);
        if (map[i].revealed)
        {
            rl::DrawText(
                rl::TextFormat("%i", map[i].mine_num),
                centerOfTempRec.x - sst::cx(rl::MeasureText(rl::TextFormat("%i", map[i].mine_num), mineNumFontSize)/2),
                centerOfTempRec.y - sst::cy((mineNumFontSize/2)),
                sst::cx(mineNumFontSize),
                Generate_Color(map[i].mine_num)
            );
            map_reveal++;
        }
        if (map[i].flagged)
        {
            rl::DrawRectangleRec(tempRec, rl::RED);
            if (map[i].flagged == map[i].is_mine)
                cor_flag++;
        }
    }
    if (cor_flag >= total_mines && map_reveal >= map_size * map_size)
        didWin = true;
    
    // rl::DrawText(rl::TextFormat("Cor_Flag: %i", cor_flag), 
    // sst::cx(1000 - rl::MeasureText(rl::TextFormat("Cor_Flag: %i", cor_flag), MINESWEEPER_STANDARD_FONT)/2),
    // sst::cy(sst::baseY - 200),
    // sst::cx(MINESWEEPER_STANDARD_FONT), rl::BLUE);

    // rl::DrawText(rl::TextFormat("map_reveal: %i", map_reveal), 
    // sst::cx(1000 - rl::MeasureText(rl::TextFormat("map_reveal: %i", map_reveal), MINESWEEPER_STANDARD_FONT)/2),
    // sst::cy(sst::baseY - 300),
    // sst::cx(MINESWEEPER_STANDARD_FONT), rl::BLUE);
}

void Minesweeper::Delete_Map()
{
    map.clear();
    tileSelected = -1;
    isDead = false;
    didWin = false;
}

void Minesweeper::Reveal_Tile(int tile)
{
    if (map[tile].revealed == true) //base case
        return;
    map[tile].revealed = true;
    if (map[tile].mine_num == 0)
    {
        int i = tile;
        if (i % map_size == 0)
        {
            if (i - map_size >= 0)
            {
                Reveal_Tile(i - map_size);
                Reveal_Tile(i - map_size + 1);
            }
            if (i + map_size <= (map_size * map_size) - 1)
            {
                Reveal_Tile(i + map_size);
                Reveal_Tile(i + map_size + 1);
            }
            Reveal_Tile(i + 1);
        } else if (i % map_size == map_size - 1) {
            if (i - map_size >= 0)
            {
                Reveal_Tile(i - map_size);
                Reveal_Tile(i - map_size - 1);
            }
            if (i + map_size <= (map_size * map_size) - 1)
            {
                Reveal_Tile(i + map_size);
                Reveal_Tile(i + map_size - 1);
            }
            Reveal_Tile(i - 1);           
        } else {
            if (i - map_size >= 0)
            {
                Reveal_Tile(i - map_size);
                Reveal_Tile(i - map_size - 1);
                Reveal_Tile(i - map_size + 1);
            }
            if (i + map_size <= (map_size * map_size) - 1)
            {
                Reveal_Tile(i + map_size);
                Reveal_Tile(i + map_size - 1);
                Reveal_Tile(i + map_size + 1);
            }
            Reveal_Tile(i - 1);
            Reveal_Tile(i + 1);  
        }
    }
}