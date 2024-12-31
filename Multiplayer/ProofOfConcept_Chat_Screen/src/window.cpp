#include "window.h"

Window::Window()
{
    font = 17;
    window = {sst::baseX/2, sst::baseY/2, CHATWINDOW_WIDTH, CHATWINDOW_HEIGHT};
    border.top_left = {window.x, window.y};
    border.bottom_right = {window.x + window.width, window.y + window.height};
    messageBox = {CHATWINDOW_MESSAGEBOX_X, CHATWINDOW_MESSAGEBOX_Y, CHATWINDOW_MESSAGE_DISPLAY_SIZE_THRESHOLD + 5, 25};
    usernameBox = {CHATWINDOW_USERNAMEBOX_X, CHATWINDOW_USERNAMEBOX_Y, CHATWINDOW_MAX_USERNAME_TEXT_SCREENSIZE + 5, 25};
    transparency = 255;
    //Produce y coordinate for each row of text
    for (int i = 0; i < CHATWINDOW_ROWS; i++) textRows[i] = window.y + 20 + 30 * i;
    message[0] = '\0';
    username[0] = '\0';
    isTyping = false;
    isTextTooLong = false;
    isConnected = false;
    curMessageSize = 0;
    curUsernameSize = 0;

    WORD wVersionRequested = MAKEWORD(2,2); //Want version 2.2 of winsock2, which is the most recent one
    wsaerr = WSAStartup(wVersionRequested, &wsaData);
    if (wsaerr != 0)
        std::cout << "Winsock not found" << std::endl;
}

void Window::updateLogic()
{
    drag();
    if (isTyping)
    {
        if (isConnected) modifyMessage();
        else modifyUsername();
    }
    else rl::SetMouseCursor(rl::MOUSE_CURSOR_DEFAULT);
    //Set to text edit mode or send the message
    if (rl::IsKeyPressed(rl::KEY_ENTER))
    {
        isTyping = !isTyping;
        if (isConnected)
        {
            if(message[0] != '\0') sendMessage();
        }
        else
        {
            if (username[0] != '\0') sendUsername();
        }
    }
    if (!isConnected)
    {
        if (rl::IsMouseButtonPressed(rl::MOUSE_BUTTON_LEFT) && rl::CheckCollisionPointRec(rl::GetMousePosition(), usernameBox)) 
            isTyping = !isTyping;
    }
    else 
    {
        if (rl::IsMouseButtonPressed(rl::MOUSE_BUTTON_LEFT) && rl::CheckCollisionPointRec(rl::GetMousePosition(), messageBox)) 
            isTyping = !isTyping;
    }
    updateNetworkLogic();
}

void Window::draw()
{
    rl::DrawRectangleRec(window, (rl::Color){0,0,0,transparency});
    if (isConnected) drawChat();
    else drawSelection();
    debugDraw();
}

void Window::debugDraw()
{
    rl::DrawText(rl::TextFormat("TextLen: %i", rl::MeasureText(message, font)), 10, 10, font, rl::BLACK);
}

void Window::drag()
{
    rl::Vector2 mouse = rl::GetMousePosition();

    if (rl::IsMouseButtonPressed(rl::MOUSE_BUTTON_LEFT))
    {
        if (rl::CheckCollisionPointRec(mouse, window))
        {
            dragOffset.x = mouse.x - window.x;
            dragOffset.y = mouse.y - window.y;
        }
    } else
    {
    }
    if (rl::IsMouseButtonDown(rl::MOUSE_BUTTON_LEFT))
    {
        if (rl::CheckCollisionPointRec(mouse, window))
        {
            //Compute new position and ensure that it is not out of bounds
            float newWindowX = mouse.x - dragOffset.x;
            float newWindowY = mouse.y - dragOffset.y;
            rl::Vector2 top_left = {newWindowX, newWindowY};
            rl::Vector2 bottom_right = {newWindowX + window.width, newWindowY + window.height};
            
            if (
                !rl::CheckCollisionPointRec(top_left, (rl::Rectangle){0, 0, sst::baseX, sst::baseY}) ||
                !rl::CheckCollisionPointRec(bottom_right, (rl::Rectangle){0, 0, sst::baseX, sst::baseY})
                ) return;

            window.x = newWindowX;
            window.y = newWindowY;
            border.top_left = top_left;
            border.bottom_right = bottom_right;
            messageBox.x = CHATWINDOW_MESSAGEBOX_X;
            messageBox.y = CHATWINDOW_MESSAGEBOX_Y;
            usernameBox.x = CHATWINDOW_USERNAMEBOX_X;
            usernameBox.y = CHATWINDOW_USERNAMEBOX_Y;
            for (int i = 0; i < CHATWINDOW_ROWS; i++)
                textRows[i] = window.y + 20 + 30 * i;
        }
    }
}

void Window::modifyMessage()
{
    rl::SetMouseCursor(rl::MOUSE_CURSOR_IBEAM);
    int key = rl::GetCharPressed(); //Read key stack
    //While there are characters, append them to the message and null terminate
    while (key > 0)
    {
        if (rl::MeasureText(message, font) > (CHATWINDOW_MAX_MESSAGE_TEXT_SCREENSIZE))
        {
            isTextTooLong = true;
            break;
        }
        if ((key >= 32) && (key <= 125) && (curMessageSize < CHATWINDOW_MESSAGE_BUFFER_LENGTH))
        {
            message[curMessageSize] = (char)key;
            message[curMessageSize + 1] = '\0';
            curMessageSize++;
            isTextTooLong = false;
        }
        key = rl::GetCharPressed();
    }
    if (rl::IsKeyPressed(rl::KEY_BACKSPACE)) removeChar();
}

void Window::sendMessage()
{
    //Attach username to message
    std::string fullMessage;
    for (int i = 0; i < curUsernameSize; i++)
        fullMessage += username[i];
    fullMessage += ": ";
    for (int i = 0; i < curMessageSize; i++)
        fullMessage += message[i];
    chat.addMessage(fullMessage.c_str());
    if (isServer) broadcastToAllClients(fullMessage.c_str());
    else sendToServer(fullMessage.c_str());
    curMessageSize = 0;
    message[0] = '\0';
}

void Window::removeChar()
{
    if (isConnected)
    {
        curMessageSize--;
        if (curMessageSize < 0) curMessageSize = 0;
        message[curMessageSize] = '\0'; 
    } else
    {
        curUsernameSize--;
        if (curUsernameSize < 0) curUsernameSize = 0;
        username[curUsernameSize] = '\0'; 
    }
}

void Window::drawChat()
{
    rl::DrawRectangleLinesEx(messageBox, 1, CHATWINDOW_TEXT_COLOUR);
    for (int i = 0; i < CHATWINDOW_ROWS; i++) rl::DrawText("-", window.x + 5, textRows[i], font, CHATWINDOW_TEXT_COLOUR);
    for (int i = 0; i < CHATWINDOW_ROWS; i++)
    {
        std::string chatMessage = chat.getMessage(i);
        if (rl::MeasureText(chatMessage.c_str(), font) > CHATWINDOW_MESSAGE_DISPLAY_SIZE_THRESHOLD)
            rl::DrawText(chatMessage.c_str(), window.x + 15, textRows[i] + 3, font - 7, CHATWINDOW_TEXT_COLOUR);
        else rl::DrawText(chatMessage.c_str(), window.x + 15, textRows[i] - 1, font, CHATWINDOW_TEXT_COLOUR);
    }
    rl::DrawText("Msg:", messageBox.x - 45, CHATWINDOW_MESSAGEBOX_TEXT_Y, font, CHATWINDOW_TEXT_COLOUR);
    if (rl::MeasureText(message, font) > CHATWINDOW_MESSAGE_DISPLAY_SIZE_THRESHOLD)
        rl::DrawText(message, messageBox.x + 5, CHATWINDOW_MESSAGEBOX_TEXT_Y + 4, font - 7, CHATWINDOW_TEXT_COLOUR);
    else rl::DrawText(message, messageBox.x + 5, CHATWINDOW_MESSAGEBOX_TEXT_Y, font, CHATWINDOW_TEXT_COLOUR);
}

void Window::drawSelection()
{
    rl::DrawText("ChatWindow", centerTextX("ChatWindow"), window.y + 20, font, CHATWINDOW_TEXT_COLOUR);
    rl::DrawText("Enter username", centerTextX("Enter username"), CHATWINDOW_USERNAMEBOX_Y - 50, font, CHATWINDOW_TEXT_COLOUR);
    rl::DrawText("to begin", centerTextX("to begin"), CHATWINDOW_USERNAMEBOX_Y - 25, font, CHATWINDOW_TEXT_COLOUR);
    rl::DrawRectangleLinesEx(usernameBox, 1, CHATWINDOW_TEXT_COLOUR);
    rl::DrawText(username, usernameBox.x + 5, CHATWINDOW_USERNAMEBOX_Y + 2, font, CHATWINDOW_TEXT_COLOUR);
}

int Window::centerTextX(const char* text)
{
    return (window.x + window.width/2) - (rl::MeasureText(text, font))/2;
}

void Window::modifyUsername()
{
    rl::SetMouseCursor(rl::MOUSE_CURSOR_IBEAM);
    int key = rl::GetCharPressed(); //Read key stack
    //While there are characters, append them to the message and null terminate
    while (key > 0)
    {
        if (rl::MeasureText(username, font) > CHATWINDOW_MAX_USERNAME_TEXT_SCREENSIZE - 15)
        {
            isTextTooLong = true;
            break;
        }
        if ((key >= 32) && (key <= 125) && curUsernameSize < CHATWINDOW_USERNAME_BUFFER_LENGTH)
        {
            username[curUsernameSize] = (char)key;
            username[curUsernameSize + 1] = '\0';
            curUsernameSize++;
            isTextTooLong = false;
        }
        key = rl::GetCharPressed();
    }

    if (rl::IsKeyPressed(rl::KEY_BACKSPACE)) removeChar();
}

void Window::sendUsername()
{
    isConnected = true;
    isTextTooLong = false;
}

void Window::close()
{
    if (isServer) server.shutDown();
    else client.shutDown();
    WSACleanup();
}

void Window::initServer()
{
    isServer = true;
    if (wsaerr != 0) return; //WSAData not initialized correctly
    if (!server.create("127.0.0.1", 20000, 1, chat))
    {
        std::cout << "Creation Failed" << std::endl;
        return;
    }
    server.listenForClients();
}

void Window::initClient()
{
    isServer = false;
    if (wsaerr != 0) return;
    if(!client.connectToServer("127.0.0.1", 20000))
    {
        std::cout << "Connection Failed" << std::endl;
        return;
    }
    client.receiveFromServer();
}

void Window::broadcastToAllClients(const std::string& msg)
{
    data.setMessage(msg.c_str());
    server.broadcast(data);
}

void Window::updateNetworkLogic()
{
    //Check for dying server here


    if (isServer)
    {

    }
    else
    {
        if(client.isThereNewData()) chat.addMessage(client.getData().getMessage());
    }
}

void Window::sendToServer(const std::string& msg)
{
    client.sendMessage(msg.c_str());
}
