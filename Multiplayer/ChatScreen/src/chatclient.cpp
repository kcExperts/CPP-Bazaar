#include "chatclient.h"

ChatClient::ChatClient()
{
    isRecvRunning = false;
    isSendRunning = false;
    isMsgRdyToSend = false;
}

bool ChatClient::connectToServer(std::string ip, int port)
{
    client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client == INVALID_SOCKET)
    {
        std::cout << "Error: " << WSAGetLastError() << std::endl;
        return false;
    }

    sockaddr_in clientService;
    clientService.sin_family = AF_INET;
    InetPtonA(AF_INET, ip.c_str(), &clientService.sin_addr.S_un);
    clientService.sin_port = htons(port);
    if (connect(client, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR)
    {
        std::cout << "Failed to connect: " << WSAGetLastError() << std::endl;
        closesocket(client);
        return false;
    }

    std::cout << "Succesfully Connected to Server" << std::endl;

    this->ip = ip;
    this->port = port;
    
    //Starts threads to communicate with server
    sendThread = std::thread(&ChatClient::sendToServer, this);

    return true;
}

void ChatClient::recvFromServer()
{
    ChatData data;
    int bytesReceived;
    isRecvRunning = true;
    while(isRecvRunning)
    {
        bytesReceived = recv(client, (char*)&data, sizeof(data), 0);
        if (bytesReceived == SOCKET_ERROR)
        {
            std::cout << "Server received failed: " << WSAGetLastError() << std::endl;
            break;
        }

        if (bytesReceived == 0)
        {
            std::cout << "You have been disconnected." << std::endl;
            break;
        }
        std::cout << "Server: " << data.getMessage() << std::endl;
    }
}

void ChatClient::sendToServer()
{
    int byteCount;
    isSendRunning = true;
    while(isSendRunning)
    {
        while(!isMsgRdyToSend) {} //Wait until message is ready to send
        int byteCount = send(client, (char*)&dataToSend, sizeof(dataToSend), 0);
        if (byteCount == SOCKET_ERROR)
        {
            std::cout << "Error in sending Message: " << WSAGetLastError() << std::endl;
        }
        isMsgRdyToSend = false;
    }
}

void ChatClient::disconnect()
{
    shutdown(client, SD_BOTH);
    closesocket(client);
}

bool ChatClient::sendMessage(char message[])
{
    //Thread has yet to finish sending message
    if (isMsgRdyToSend)
        return false;
    bool out = dataToSend.setMessage(message);
    //Halt new messages from being created until thread resets flag
    isMsgRdyToSend = true;
    return out;
}

