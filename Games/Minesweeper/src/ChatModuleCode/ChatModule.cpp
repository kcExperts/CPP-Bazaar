#include "ChatModule.h"

ChatModule::ChatModule(int fps)
{
    ChatWindow = rl::Rectangle{CHATWINDOW_START_X, CHATWINDOW_START_Y, CHATWINDOW_WIDTH, CHATWINDOW_HEIGHT};
    WindowColor = rl::BLACK;
    allowDrag = true;
    currentMenu = Init;
    isServer = false;
    InitLoadingImage();
    areButtonsInitialized = false;
    usernameLength = 0;
    messageLength = 0;
    portLength = 0;
    ipLength = 0;
    isTyping = false;
    username[0] = '\0';
    message[0] = '\0';
    port[0] = '\0';
    ip[0] = '\0';
    wasNetworkCreated = Null;
    Network_InitializeWSA(gen_err);
    program_fps = fps;
    size_t count = 0;
    displayError = false;
    timeToLeaveMenu = false;
    isInitializedByOtherProgram = false;
}

ChatModule::ChatModule(int fps, bool isServer, std::string server_ip, std::string server_port, std::string chat_username)
{
    ChatWindow = rl::Rectangle{CHATWINDOW_START_X + 200, CHATWINDOW_START_Y, CHATWINDOW_WIDTH, CHATWINDOW_HEIGHT};
    WindowColor = rl::BLACK;
    allowDrag = true;
    this->isServer = isServer;
    InitLoadingImage();
    areButtonsInitialized = false;
    usernameLength = chat_username.length();
    messageLength = 0;
    portLength = server_port.length();
    if (isServer) {ipLength = 0;}
    else {ipLength = server_ip.length();}
    isTyping = false;
    std::strncpy(username, chat_username.c_str(), sizeof(username) - 1);
    std::strncpy(port, server_port.c_str(), sizeof(port) - 1);
    std::strncpy(ip, server_ip.c_str(), sizeof(ip) - 1);
    username[sizeof(username) - 1] = '\0';
    message[0] = '\0';
    port[sizeof(port) - 1] = '\0';
    if (isServer) {ip[0] = '\0';}
    else {ip[sizeof(ip) - 1] = '\0';}
    wasNetworkCreated = Null;
    Network_InitializeWSA(gen_err);
    program_fps = fps;
    size_t count = 0;
    displayError = false;
    timeToLeaveMenu = false;
    if (isServer) {currentMenu = ServerCreation;}
    else {currentMenu = ClientCreation;}
    isInitializedByOtherProgram = true;
    didConnectionWork = -1;
}

int ChatModule::getDidConnectionWork()
{
    return didConnectionWork;
}

ChatModule::~ChatModule() //Need better implementation
{
    if (currentMenu == Chat)
    {
        if (isServer) {Network_Server_Shutdown(server_locks, server_info);}
        else {Network_Client_Shutdown(client_locks, client_info);} 
    }
};

void ChatModule::Draw()
{
    rl::DrawRectangle(sst::cxf(ChatWindow.x), sst::cyf(ChatWindow.y), sst::cxf(ChatWindow.width), sst::cyf(ChatWindow.height), WindowColor);
    switch (currentMenu)
    {
    case ServerCreation:
        if (!areButtonsInitialized) InitializeServerCreation();
        DrawLoading();
        break;
    case ClientCreation:
        if (!areButtonsInitialized) InitializeClientCreation();
        DrawLoading();
        break;
    case Chat:
        if (!areButtonsInitialized) InitializeChatScreen();
        DrawChatScreen();
        break;
    }
}

void ChatModule::UpdateState()
{
    if (rl::IsKeyPressed(rl::KEY_D) && !isTyping) allowDrag = !allowDrag;
    Drag();
    switch (currentMenu)
    {
    case ServerCreation:
        UpdateServerCreation();
        break;
    case ClientCreation:
        UpdateClientCreation();
        break;
    case Chat:
        UpdateChatScreen();
        break;
    }
}

void ChatModule::InitializeChatScreen()
{
    client_vec_prev_size = 0;
    areButtonsInitialized = true;
    msgFontSize = STANDARD_FONT_SIZE;
    errorMsgOut.clear();
    ButtonInfoInit();
    buttonInfo.hasTextBox = false;
    buttonInfo.locationOrientation = Center;
    rl::Rectangle rec = CenterRect(CHATWINDOW_WIDTH - 10, CHATWINDOW_HEIGHT - 70);
    rec.y -=30;
    buttonInfo.rectangle = rec;
    buttons["ChatWindowBounds"] = buttonInfo;
    buttonInfo.hasTextBox = true;
    buttonInfo.text = "Msg";
    buttonInfo.textBox.color = rl::WHITE;
    buttonInfo.textBox.type = Message;
    rec = CenterRect(CHATWINDOW_MSG_BOX_WIDTH, CHATWINDOW_DEFAULT_TEXTBOX_HEIGHT);
    rec.y += 50;
    rec.x += rl::MeasureText("Msg", STANDARD_FONT_SIZE)/2;
    buttonInfo.textBox.location = rec;
    buttons["MessageBox"] = buttonInfo;
}

void ChatModule::DrawChatScreen()
{
    if (!timeToLeaveMenu)
    {
        DrawRecLinesFromButtonInfo(buttons.at("ChatWindowBounds"), 2);
        DrawChatHistory();
        DrawTextLeftOfRect(buttons.at("MessageBox"), 2);
        DrawTextBox(buttons.at("MessageBox"), Message, false);
        if (fps_counter < 2 * program_fps)
        {
            DrawTextCenteredAtXY(errorMsgOut, rl::MeasureText(errorMsgOut.c_str(), 10)/2, CHATWINDOW_HEIGHT, 5, 10, rl::RED);
        } else {
            displayError = false;
            fps_counter = 0;
            errorMsgOut.clear();
        }
        std::scoped_lock lock(server_locks.client_vector_mtx, server_locks.client_vector_capacity_mtx);
        rl::DrawText(
            rl::TextFormat("Clients Connected: %i", server_info.client_vector.size()),
            sst::cx(CHATWINDOW_WIDTH - rl::MeasureText(rl::TextFormat("Clients Connected: %i", server_info.client_vector.size()) ,STANDARD_FONT_SIZE - 5) - 10 + CHATWINDOW_OFFSET_X),
            sst::cy(CHATWINDOW_HEIGHT - 60 + CHATWINDOW_DEFAULT_TEXTBOX_HEIGHT + CHATWINDOW_OFFSET_Y),
            sst::cx(STANDARD_FONT_SIZE - 5),
            rl::RED
        );
    } 
    else {DrawLoading();}
}

void ChatModule::UpdateChatScreen()
{
    if (!timeToLeaveMenu)
    {
        int font = STANDARD_FONT_SIZE;
        msgFontSize = font;
        int textLength = rl::MeasureText(message, font);
        if (!(textLength < (CHATWINDOW_MSG_BOX_WIDTH - 30)))
        {
            msgFontSize = STANDARD_FONT_SIZE - 3;
            font -= 3;
            textLength = rl::MeasureText(message, font);
            if (!(textLength < (CHATWINDOW_MSG_BOX_WIDTH - 30))) msgFontSize = STANDARD_FONT_SIZE - 7;
        }
        {   //errorMsgOut updating logic
            if (displayError) fps_counter++;
            std::string temp = Network_GetLastError(gen_err);
            if (isServer)
            {
                std::scoped_lock lock(server_locks.client_vector_mtx, server_locks.client_vector_capacity_mtx);
                size_t clientsInServer = server_info.client_vector.size();
                if (client_vec_prev_size < clientsInServer) temp = " A client has joined the server";
                client_vec_prev_size = clientsInServer;
            }
            if (temp.size() != 0)
            {
                displayError = true;
                errorMsgOut = temp;
                fps_counter = 0;
            }
        }
        ModifyTextBox();
        CheckButtonCollision();
        if (rl::IsMouseButtonPressed(rl::MOUSE_BUTTON_LEFT))
        {
            bool isE = buttons.at("MessageBox").textBox.isEditing;
            if (buttons.at("MessageBox").isMouseHovering && ((!isTyping && !isE) || (isTyping && isE)))
            {
                buttons.at("MessageBox").textBox.isEditing = !buttons.at("MessageBox").textBox.isEditing;
                isTyping = !isTyping;
            }
            //CHECK FOR CLOSING MENU
        }
        if (rl::IsKeyPressed(rl::KEY_ENTER))
        {
            if (messageLength > 0)
            {
                std::lock_guard lock(server_locks.sending_data_mtx);
                //Broadcasts to all clients / sends to the server
                if (isServer)
                {
                    server_info.dataToSend.setMessage(message);
                    Network_Server_SendMsg(server_locks);
                    DataHandler(server_info.dataToSend);
                } else {
                    client_info.dataToSend.setMessage(message);
                    Network_Client_Updates(client_locks);
                    DataHandler(client_info.dataToSend);
                }
                messageLength = 0;
                message[0] = '\0';
            }
        }
        if (isServer) Network_Server_Updates(server_locks, server_info);
    } else {
        if (!isThreadDone) {UpdateLoading();} 
        else {
            intermediateThread.join();
            messageLength = 0;
            message[0] = '\0';
            currentMenu = Select;
            timeToLeaveMenu = false;
            areButtonsInitialized = false;
            messageHistory.clear();
        }
    }
}

void ChatModule::CreateClient()
{
    Network_Client_Init(5, client_info);
    client_info.dataToSend.setUsername(username);
    client_info.data_Handler_Func = [this](const ChatObject& data) {this->DataHandler(data);};
    if (portLength == 0)
    {
        wasNetworkCreated = False;
        std::unique_lock<std::mutex> lock(gen_err.msg_buffer_mtx);
        gen_err.msg_buffer = "Enter valid port number";
        intermediateThread_cv.notify_all();
        return;
    }
    Network_Client_Set_Port(port, client_info);
    if (ipLength == 0)
    {
        wasNetworkCreated = False;
        std::unique_lock<std::mutex> lock(gen_err.msg_buffer_mtx);
        gen_err.msg_buffer = "Enter valid ip address";
        intermediateThread_cv.notify_all();
        return;
    }
    Network_Client_Set_Ip(ip, client_info);
    if (!Network_Client_CreateSocket(client_locks, client_info))
    {
        wasNetworkCreated = False;
        std::unique_lock<std::mutex> lock(gen_err.msg_buffer_mtx);
        gen_err.msg_buffer = "Socket creation failed";
        intermediateThread_cv.notify_all();
        return;
    }
    if (std::stoi(port) == 0) //Loopback connection
    {
        if (!Network_Client_LoopbackConnect(gen_err, client_locks, client_info))
        {
            wasNetworkCreated = False;
            intermediateThread_cv.notify_all();
            return;
        }
    } else {
        if (!Network_Client_ConnectToServer(gen_err, client_locks, client_info))
        {
            wasNetworkCreated = False;
            intermediateThread_cv.notify_all();
            return;
        }
    }
    Network_Client_InitThreads(gen_err, client_locks, client_info);
    wasNetworkCreated = True;
    intermediateThread_cv.notify_all();
}

void ChatModule::IntermediateClientCreationThread()
{
    isThreadDone = false;
    loading_thread = std::thread([this]{CreateClient();});
    std::unique_lock lock(intermediateThread_mtx);
    intermediateThread_cv.wait(lock);
    loading_thread.join();
    isThreadDone = true;
}

void ChatModule::InitializeClientCreation()
{
    intermediateThread = std::thread([this]{IntermediateClientCreationThread();});
    buttons.clear();
    areButtonsInitialized = true;
}

void ChatModule::UpdateClientCreation()
{
    if (!isThreadDone) UpdateLoading();
    else
    {
        intermediateThread.join();
        if (wasNetworkCreated == True) 
        {
            didConnectionWork = 1;
            areButtonsInitialized = false;
            currentMenu = Chat;
        } else if (wasNetworkCreated == False){
            didConnectionWork = 0;
            areButtonsInitialized = false;
        }
    }
}

void ChatModule::IntermediateServerLeavingThread()
{
    isThreadDone = false;
    loading_thread = std::thread([this]{
        Network_Server_Shutdown(server_locks, server_info);
        intermediateThread_cv.notify_all();
        });
    std::unique_lock lock(intermediateThread_mtx);
    intermediateThread_cv.wait(lock);
    loading_thread.join();
    isThreadDone = true;    
}

void ChatModule::IntermediateClientLeavingThread()
{
    isThreadDone = false;
    loading_thread = std::thread([this]{
        Network_Client_Shutdown(client_locks, client_info);
        intermediateThread_cv.notify_all();
        });
    std::unique_lock lock(intermediateThread_mtx);
    intermediateThread_cv.wait(lock);
    loading_thread.join();
    isThreadDone = true;    
}

void ChatModule::DataHandler(const ChatObject& data) //Recycled code from previous chat screen
{
    const std::size_t len = MAX_USERNAME_LEN + MAX_MSG_LEN + 1;
    std::lock_guard lockA(messageHistory_mtx);
    std::array<char, len> storedMessage; //Temporary container
    std::strncpy(storedMessage.data(), data.getUsername().c_str(), MAX_USERNAME_LEN); //Add username
    std::strncat(storedMessage.data(), ": ", len - std::strlen(storedMessage.data()));
    std::strncat(storedMessage.data(), data.getMessage().c_str(), len - std::strlen(storedMessage.data())); //Copy over message
    storedMessage[MAX_MSG_LEN] = '\0'; //Ensure null termination
    messageHistory.push_back(storedMessage); //Store message
    if (messageHistory.size() > MAX_MESSAGE_HISTORY_STORAGE_SIZE) messageHistory.pop_front(); //Remove oldest message
}

void ChatModule::CreateServer()
{
    Network_Server_Init(MAX_CHAT_CONNECTIONS, server_locks, server_info);
    server_info.dataToSend.setUsername(username);
    server_info.data_Handler_Func = [this](const ChatObject& data) {this->DataHandler(data);}; //Non-static member function requires lambda
    if (portLength == 0)
    {
        wasNetworkCreated = False;
        std::unique_lock<std::mutex> lock(gen_err.msg_buffer_mtx);
        gen_err.msg_buffer = "Enter valid port number";
        intermediateThread_cv.notify_all();
        return;
    }
    //Debug Port
    Network_Server_Set_Port(port, server_info);
    if (!Network_Server_CreateSocket(server_info))
    {
        wasNetworkCreated = False;
        std::unique_lock<std::mutex> lock(gen_err.msg_buffer_mtx);
        gen_err.msg_buffer = "Socket creation failed";
        intermediateThread_cv.notify_all();
        return;
    }
    if (std::stoi(port) == 0) //Loopback port option
    {
        if (!Network_Server_LoopbackBind(gen_err, server_info))
        {
            wasNetworkCreated = False;
            intermediateThread_cv.notify_all();
            return;
        }
    } else {
        if (!Network_Server_Bind(gen_err, server_info))
        {
            wasNetworkCreated = False;
            intermediateThread_cv.notify_all();
            return;
        }  
    }
    if (!Network_Server_InitThreads(gen_err, server_locks, server_info))
    {
        wasNetworkCreated = False;
        Network_Server_Shutdown(server_locks, server_info);
        intermediateThread_cv.notify_all();
        return;
    }
    wasNetworkCreated = True;
    intermediateThread_cv.notify_all();
}

void ChatModule::IntermediateServerCreationThread()
{
    isThreadDone = false;
    loading_thread = std::thread([this]{CreateServer();});
    std::unique_lock lock(intermediateThread_mtx);
    intermediateThread_cv.wait(lock);
    loading_thread.join();
    isThreadDone = true;
}

void ChatModule::InitializeServerCreation()
{
    intermediateThread = std::thread([this]{IntermediateServerCreationThread();});
    buttons.clear();
    areButtonsInitialized = true;
}

void ChatModule::UpdateServerCreation()
{
    if (!isThreadDone) UpdateLoading();
    else 
    {
        intermediateThread.join();
        if (wasNetworkCreated == True) 
        {
            didConnectionWork = 1;
            areButtonsInitialized = false;
            currentMenu = Chat;
        } else if (wasNetworkCreated == False){
            didConnectionWork = 0;
            areButtonsInitialized = false;
        }
    }
}

void ChatModule::DrawLoading() const
{
    // What the heck is going on with the line below
    rl::Rectangle loadingDest = {
        sst::cxf(GetCenterX() + T_loading.textureCenter.x - 100 / 2 + ChatWindow.x),
        sst::cyf(GetCenterY() + T_loading.textureCenter.y - 100 / 2 + ChatWindow.y),
        sst::cxf(100),
        sst::cyf(100)};
    rl::DrawTexturePro(T_loading.texture, T_loading.sourceRec, loadingDest,
        rl::Vector2{sst::cxf(T_loading.textureCenter.x), sst::cyf(T_loading.textureCenter.y)}, T_loading.currentAngle, rl::WHITE);
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

void ChatModule::Drag()
{
    if (!allowDrag) return;
    rl::Vector2 mousePos = rl::GetMousePosition();
    mousePos.x = mousePos.x * ((float)sst::baseX/(float)rl::GetScreenWidth()); //Resize to use base 1280 x 720
    mousePos.y = mousePos.y * ((float)sst::baseY/(float)rl::GetScreenHeight()); //Resize to use base 1280 x 720
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
        if (!(newWindowX <= (float)sst::baseY || newWindowX + ChatWindow.width >= sst::baseX))
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
        sst::cxf(info.textBox.location.x + CHATWINDOW_OFFSET_X),
        sst::cyf(info.textBox.location.y + CHATWINDOW_OFFSET_Y),
        sst::cxf(info.textBox.location.width),
        sst::cyf(info.textBox.location.height)};
    if (!info.hasTextBox)
    {
    rl::DrawText(
        info.text.c_str(),
        sst::cx((info.textBox.location.x + info.textBox.location.width / 2) - rl::MeasureText(info.text.c_str(), info.font) / 2 + CHATWINDOW_OFFSET_X),
        sst::cy(info.textBox.location.y - info.font - padding + CHATWINDOW_OFFSET_Y),
        sst::cx(info.font),
        info.isMouseHovering ? info.selectColor : info.nonSelectColor);
        rl::DrawRectangleLinesEx(tempRec, sst::cxf(info.textBox.thickness), info.textBox.color);
    } else {
        rl::DrawText(
        info.text.c_str(),
        sst::cx((info.textBox.location.x + info.textBox.location.width / 2) - rl::MeasureText(info.text.c_str(), info.font) / 2 + CHATWINDOW_OFFSET_X),
        sst::cy(info.textBox.location.y - info.font - padding + CHATWINDOW_OFFSET_Y),
        sst::cx(info.font),
        info.nonSelectColor);
        rl::DrawRectangleLinesEx(tempRec, sst::cxf(info.textBox.thickness), info.textBox.isEditing ? info.selectColor : (info.isMouseHovering ? info.selectColor : info.nonSelectColor));
    }
}
void ChatModule::DrawTextBelowRect(const ChatModule_TextInfo &info, int padding) const
{
    rl::Rectangle tempRec = {
        sst::cxf(info.textBox.location.x + CHATWINDOW_OFFSET_X),
        sst::cyf(info.textBox.location.y + CHATWINDOW_OFFSET_Y),
        sst::cxf(info.textBox.location.width),
        sst::cyf(info.textBox.location.height)};
    if (!info.hasTextBox)
    {
        rl::DrawText(
        info.text.c_str(),
        sst::cx((info.textBox.location.x + info.textBox.location.width / 2) - rl::MeasureText(info.text.c_str(), info.font) / 2+ CHATWINDOW_OFFSET_X),
        sst::cy(info.textBox.location.y + info.textBox.location.height + padding+ CHATWINDOW_OFFSET_Y),
        sst::cx(info.font),
        info.isMouseHovering ? info.selectColor : info.nonSelectColor);
        rl::DrawRectangleLinesEx(tempRec, sst::cxf(info.textBox.thickness), info.textBox.color);
    } else {
        rl::DrawText(
        info.text.c_str(),
        sst::cx((info.textBox.location.x + info.textBox.location.width / 2) - rl::MeasureText(info.text.c_str(), info.font) / 2+ CHATWINDOW_OFFSET_X),
        sst::cy(info.textBox.location.y + info.textBox.location.height + padding+ CHATWINDOW_OFFSET_Y),
        sst::cx(info.font),
        info.nonSelectColor);
        rl::DrawRectangleLinesEx(tempRec, sst::cxf(info.textBox.thickness), info.textBox.isEditing ? info.selectColor : (info.isMouseHovering ? info.selectColor : info.nonSelectColor));
    }
}
void ChatModule::DrawTextLeftOfRect(const ChatModule_TextInfo &info, int padding) const
{
    rl::Rectangle tempRec = {
        sst::cxf(info.textBox.location.x + CHATWINDOW_OFFSET_X),
        sst::cyf(info.textBox.location.y + CHATWINDOW_OFFSET_Y),
        sst::cxf(info.textBox.location.width),
        sst::cyf(info.textBox.location.height)};
    if (!info.hasTextBox)
    {
        rl::DrawText(
        info.text.c_str(),
        sst::cx(info.textBox.location.x - rl::MeasureText(info.text.c_str(), info.font) - padding+ CHATWINDOW_OFFSET_X),
        sst::cy((info.textBox.location.y + info.textBox.location.height / 2) - info.font / 2+ CHATWINDOW_OFFSET_Y),
        sst::cx(info.font),
        info.isMouseHovering ? info.selectColor : info.nonSelectColor);
        rl::DrawRectangleLinesEx(info.textBox.location, sst::cxf(info.textBox.thickness), info.textBox.color);
    } else {
        rl::DrawText(
        info.text.c_str(),
        sst::cx(info.textBox.location.x - rl::MeasureText(info.text.c_str(), info.font) - padding+ CHATWINDOW_OFFSET_X),
        sst::cy((info.textBox.location.y + info.textBox.location.height / 2) - info.font / 2+ CHATWINDOW_OFFSET_Y),
        sst::cx(info.font),
        info.nonSelectColor);
        rl::DrawRectangleLinesEx(tempRec, sst::cxf(info.textBox.thickness), info.textBox.isEditing ? info.selectColor : (info.isMouseHovering ? info.selectColor : info.nonSelectColor));
    }
}
void ChatModule::DrawTextRightOfRect(const ChatModule_TextInfo &info, int padding) const
{
    rl::Rectangle tempRec = {
        sst::cxf(info.textBox.location.x + CHATWINDOW_OFFSET_X),
        sst::cyf(info.textBox.location.y + CHATWINDOW_OFFSET_Y),
        sst::cxf(info.textBox.location.width),
        sst::cyf(info.textBox.location.height)};
    if (!info.hasTextBox)
    {
        rl::DrawText(
        info.text.c_str(),
        sst::cx(info.textBox.location.x + info.textBox.location.width + padding + CHATWINDOW_OFFSET_X),
        sst::cy((info.textBox.location.y + info.textBox.location.height / 2) - info.font / 2 + CHATWINDOW_OFFSET_Y),
        sst::cx(info.font),
        info.isMouseHovering ? info.selectColor : info.nonSelectColor);
        rl::DrawRectangleLinesEx(info.textBox.location, sst::cxf(info.textBox.thickness), info.textBox.color);
    } else {
        rl::DrawText(
        info.text.c_str(),
        sst::cx(info.textBox.location.x + info.textBox.location.width + padding + CHATWINDOW_OFFSET_X),
        sst::cy((info.textBox.location.y + info.textBox.location.height / 2) - info.font / 2 + CHATWINDOW_OFFSET_Y),
        sst::cx(info.font),
        info.nonSelectColor);
        rl::DrawRectangleLinesEx(tempRec, sst::cxf(info.textBox.thickness), info.textBox.isEditing ? info.selectColor : (info.isMouseHovering ? info.selectColor : info.nonSelectColor));
    }
}

void ChatModule::DrawTextOnPoint(const ChatModule_TextInfo &info) const
{
    rl::DrawText(
        info.text.c_str(),
        sst::cx(info.textLocationPoint.x - rl::MeasureText(info.text.c_str(), info.font) / 2 + CHATWINDOW_OFFSET_X),
        sst::cy(info.textLocationPoint.y - (info.font / 2) + CHATWINDOW_OFFSET_Y),
        sst::cx(info.font),
        info.isMouseHovering ? info.selectColor : info.nonSelectColor);
}

void ChatModule::DrawTextLeftOfPoint(const ChatModule_TextInfo &info) const
{
    rl::DrawText(
        info.text.c_str(),
        sst::cx(info.textLocationPoint.x - rl::MeasureText(info.text.c_str(), info.font) + CHATWINDOW_OFFSET_X),
        sst::cy(info.textLocationPoint.y - (info.font / 2) + CHATWINDOW_OFFSET_Y),
        sst::cx(info.font),
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
    errorMsgOut = "";
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
            (rl::Rectangle){sst::cxf(button.second.textBox.location.x + CHATWINDOW_OFFSET_X),
            sst::cyf(button.second.textBox.location.y + CHATWINDOW_OFFSET_Y),
            sst::cxf(button.second.textBox.location.width), 
            sst::cyf(button.second.textBox.location.height)}))
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
            case Message:
                while (key > 0)
                {
                    if (rl::MeasureText(message, msgFontSize) > CHATWINDOW_MSG_BOX_WIDTH - 20) break; //Disallow long messages
                    if ((key >= 32) && (key <= 125) && (key != 92) && messageLength < MAX_MSG_LEN)
                    {
                        message[messageLength] = (char)key;
                        message[messageLength + 1] = '\0';
                        messageLength++;
                    }
                    key = rl::GetCharPressed();
                }
                if (rl::IsKeyPressed(rl::KEY_BACKSPACE))
                {
                    messageLength--;
                    if (messageLength < 0) messageLength = 0;
                    message[messageLength] = '\0';
                }
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
                    sst::cx(info.textBox.location.x + CHATWINDOW_OFFSET_X + info.textBox.location.width/2 - rl::MeasureText(username, info.font)/2),
                    sst::cy(info.textBox.location.y + (float)(info.textBox.location.height - info.font)/2 + CHATWINDOW_OFFSET_Y),
                    sst::cx(info.font), info.nonSelectColor);
            } else {
                rl::DrawText(
                    username,
                    sst::cx(info.textBox.location.x + CHATWINDOW_OFFSET_X + 5),
                    sst::cy(info.textBox.location.y + (float)(info.textBox.location.height - info.font)/2 + CHATWINDOW_OFFSET_Y),
                    sst::cx(info.font), info.nonSelectColor);
            }
            break;
        case Port:
            if (isCentered)
            {
                rl::DrawText(
                    port,
                    sst::cx(info.textBox.location.x + CHATWINDOW_OFFSET_X + info.textBox.location.width/2 - rl::MeasureText(port, info.font)/2),
                    sst::cy(info.textBox.location.y + (float)(info.textBox.location.height - info.font)/2 + CHATWINDOW_OFFSET_Y),
                    sst::cx(info.font), info.nonSelectColor);
            } else {
                rl::DrawText(
                    port,
                    sst::cx(info.textBox.location.x + CHATWINDOW_OFFSET_X + 5),
                    sst::cy(info.textBox.location.y + (float)(info.textBox.location.height - info.font)/2 + CHATWINDOW_OFFSET_Y),
                    sst::cx(info.font), info.nonSelectColor);
            }
            break;
        case Ip:
            if (isCentered)
            {
                rl::DrawText(
                    ip,
                    sst::cx(info.textBox.location.x + CHATWINDOW_OFFSET_X + info.textBox.location.width/2 - rl::MeasureText(ip, info.font)/2),
                    sst::cy(info.textBox.location.y + (float)(info.textBox.location.height - info.font)/2 + CHATWINDOW_OFFSET_Y),
                    sst::cx(info.font), info.nonSelectColor);
            } else {
                rl::DrawText(
                    ip,
                    sst::cx(info.textBox.location.x + CHATWINDOW_OFFSET_X + 5),
                    sst::cy(info.textBox.location.y + (float)(info.textBox.location.height - info.font)/2 + CHATWINDOW_OFFSET_Y),
                    sst::cx(info.font), info.nonSelectColor);
            }
            break;
        case Message:
            if (isCentered)
            {
                rl::DrawText(
                    message,
                    sst::cx(info.textBox.location.x + CHATWINDOW_OFFSET_X + info.textBox.location.width/2 - rl::MeasureText(message, msgFontSize)/2),
                    sst::cy(info.textBox.location.y + (float)(info.textBox.location.height - msgFontSize)/2 + CHATWINDOW_OFFSET_Y),
                    sst::cx(msgFontSize), info.nonSelectColor);
            } else {
                rl::DrawText(
                    message,
                    sst::cx(info.textBox.location.x + CHATWINDOW_OFFSET_X + 5),
                    sst::cy(info.textBox.location.y + (float)(info.textBox.location.height - msgFontSize)/2 + CHATWINDOW_OFFSET_Y),
                    sst::cx(msgFontSize), info.nonSelectColor);
            }
            break;

    }
}

void ChatModule::DrawTextCenteredAtXY(const std::string &text, int x, int y, int font, int offsetAbove, rl::Color color) const
{
    rl::DrawText(text.c_str(),
    sst::cx(x + CHATWINDOW_OFFSET_X - (float)rl::MeasureText(text.c_str(), font)/2),
    sst::cy(y + CHATWINDOW_OFFSET_Y - font/2 - offsetAbove), sst::cx(font), color);
}

void ChatModule::DrawRecFromButtonInfo(const ChatModule_TextInfo& info) const
{
    rl::DrawRectangle(
        sst::cx(info.rectangle.x + CHATWINDOW_OFFSET_X), 
        sst::cy(info.rectangle.y + CHATWINDOW_OFFSET_Y), 
        sst::cx(info.rectangle.width), 
        sst::cy(info.rectangle.height),
        info.nonSelectColor);
}

void ChatModule::DrawRecLinesFromButtonInfo(const ChatModule_TextInfo& info, float linethickness) const
{
    rl::Rectangle rec = {
        sst::cxf(info.rectangle.x + CHATWINDOW_OFFSET_X),
        sst::cyf(info.rectangle.y + CHATWINDOW_OFFSET_Y), 
        sst::cxf(info.rectangle.width), 
        sst::cyf(info.rectangle.height)};
    rl::DrawRectangleLinesEx(rec, sst::cxf(linethickness), info.nonSelectColor);
}

//Assuming DrawTextOnPoint is used
void ChatModule::GetRectFromPoint_Center(const ChatModule_TextInfo &info, rl::Rectangle& rec)
{
    rec.x = sst::cxf(info.textLocationPoint.x - rl::MeasureText(info.text.c_str(), info.font) / 2 + CHATWINDOW_OFFSET_X);
    rec.y = sst::cyf(info.textLocationPoint.y - info.font / 2 + CHATWINDOW_OFFSET_Y);
    rec.width = sst::cxf((float)rl::MeasureText(info.text.c_str(), info.font));
    rec.height = sst::cyf((float)info.font);
}

void ChatModule::GetRectFromPoint_Top(const ChatModule_TextInfo &info, rl::Rectangle& rec)
{
    
}

void ChatModule::GetRectFromPoint_Left(const ChatModule_TextInfo &info, rl::Rectangle& rec)
{
    rec.x = sst::cxf(info.textLocationPoint.x - rl::MeasureText(info.text.c_str(), info.font) + CHATWINDOW_OFFSET_X);
    rec.y = sst::cyf(info.textLocationPoint.y - info.font / 2 + CHATWINDOW_OFFSET_Y);
    rec.width = sst::cxf((float)rl::MeasureText(info.text.c_str(), info.font));
    rec.height = sst::cyf((float)info.font);
}

void ChatModule::GetRectFromPoint_Right(const ChatModule_TextInfo &info, rl::Rectangle& rec)
{

}

void ChatModule::GetRectFromPoint_Bottom(const ChatModule_TextInfo &info, rl::Rectangle& rec)
{

}

void ChatModule::DrawChatHistory()
{
    std::lock_guard lock(messageHistory_mtx);
    int i = 0;
    for (const auto& message : messageHistory)
    {
        //Determine font to draw to screen with
        int font = STANDARD_FONT_SIZE - 2;
        int textLength = rl::MeasureText(message.data(), font);
        if (!(textLength < (CHATWINDOW_MSG_BOX_WIDTH - 30)))
        {
            font -= 6;
            textLength = rl::MeasureText(message.data(), font);
            if (!(textLength < (CHATWINDOW_MSG_BOX_WIDTH - 30))) font -= 8;
        }
        //Draw to screen
        rl::DrawText(message.data(), sst::cx(10 + CHATWINDOW_OFFSET_X), sst::cy(20*i+ 10+  CHATWINDOW_OFFSET_Y), sst::cx(font), rl::WHITE);
        i++;
    }
}