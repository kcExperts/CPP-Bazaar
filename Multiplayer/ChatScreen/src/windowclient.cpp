#include "windowclient.h"

WindowClient::WindowClient()
{
    isRecvRunning = false;
    newData = false;
    failure = false;
    isSendRunning = false;
    isMsgRdyToSend = false;
}

WindowClient::~WindowClient()
{
    shutDown();
}

bool WindowClient::connectToServer(std::string ip, int port)
{
    //Create socket
    client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client == INVALID_SOCKET) return false;

    //Connect to server
    sockaddr_in clientService;
    clientService.sin_family = AF_INET;
    InetPtonA(AF_INET, ip.c_str(), &clientService.sin_addr.S_un);
    clientService.sin_port = htons(port);
    if (connect(client, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR)
    {
        closesocket(client);
        return false;
    }
    //Starts thread to communicate with server
    sendThread = std::thread(&WindowClient::sendToServer, this);
    return true;
}

void WindowClient::receiveFromServer()
{
    isRecvRunning = true;
    recvThread = std::thread([this]() {
        int bytesReceived;
        while (isRecvRunning)
        {
            //Prepare the read set for `select`
            fd_set readSet;
            FD_ZERO(&readSet);
            FD_SET(client, &readSet);

            //Set a timeout for `select`
            timeval timeout;
            timeout.tv_sec = 1;  // Timeout in seconds
            timeout.tv_usec = 0; // Timeout in microseconds

            // Wait for activity on the socket
            int activity = select(0, &readSet, NULL, NULL, &timeout);
            if (activity == SOCKET_ERROR) failure = true;

            //Check if the socket is ready to be read
            if (activity > 0 && FD_ISSET(client, &readSet))
            {
                //Receive data from the server
                bytesReceived = recv(client, (char*)&data, sizeof(data), 0);
                if (bytesReceived == SOCKET_ERROR)
                {
                    std::cerr << "Receiving from server failed: " << WSAGetLastError() << std::endl;
                    failure = true;
                    break;
                }
                if (bytesReceived == 0)
                {
                    std::cout << "Server has disconnected." << std::endl;
                    failure = true;
                    break;
                }
                newData = true;
                //Process the received data
                //std::cout << "Bytes Received: " << bytesReceived << std::endl;
                //std::cout << "Message from Server: " << data.getMessage() << std::endl;
            }
            //If no activity, continue the loop without blocking
        }
    });
}

bool WindowClient::isWorking() {return !failure;}

bool WindowClient::isThereNewData() {return newData;}

void WindowClient::shutDown()
{
    isRecvRunning = false;
    if (recvThread.joinable()) 
        recvThread.join();
    isSendRunning = false;
    if (sendThread.joinable())
        sendThread.join();
    shutdown(client, SD_BOTH);
    closesocket(client);
}

const WindowData& WindowClient::getData() {newData = false; return data;}

void WindowClient::sendMessage(const char message[])
{
    //Thread has yet to finish sending message //ADD
    dataToSend.setMessage(message);
    //Halt new messages from being created until thread resets flag
    isMsgRdyToSend = true;
}

void WindowClient::sendToServer()
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