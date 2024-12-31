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
    isRecvRunning = true;

    recvThread = std::thread([this]() {
        ChatData data;
        int bytesReceived;

        while (isRecvRunning)
        {
            // Prepare the read set for `select`
            fd_set readSet;
            FD_ZERO(&readSet);
            FD_SET(client, &readSet);

            // Set a timeout for `select`
            timeval timeout;
            timeout.tv_sec = 1;  // Timeout in seconds
            timeout.tv_usec = 0; // Timeout in microseconds

            // Wait for activity on the socket
            int activity = select(0, &readSet, NULL, NULL, &timeout);

            if (activity == SOCKET_ERROR)
            {
                std::cerr << "Select failed: " << WSAGetLastError() << std::endl;
                break;
            }
            //Check if the socket is ready to be read
            if (activity > 0 && FD_ISSET(client, &readSet))
            {
                //Receive data from the server
                bytesReceived = recv(client, (char*)&data, sizeof(data), 0);
                if (bytesReceived == SOCKET_ERROR)
                {
                    std::cerr << "Receiving from server failed: " << WSAGetLastError() << std::endl;
                    break;
                }
                if (bytesReceived == 0)
                {
                    std::cout << "Server has disconnected." << std::endl;
                    break;
                }
                //Process the received data
                std::cout << "Bytes Received: " << bytesReceived << std::endl;
                std::cout << "Message from Server: " << data.getMessage() << std::endl;
            }
            //If no activity, continue the loop without blocking
        }
    });
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
    isSendRunning = false;
    isRecvRunning = false;
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

