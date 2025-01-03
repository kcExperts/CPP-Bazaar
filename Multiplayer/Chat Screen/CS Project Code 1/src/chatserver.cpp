#include "chatserver.h"

ChatServer::ChatServer()
{
    isListenRunning = false;
    totalConnections = 0;
}

ChatServer::~ChatServer()
{
    stopThread();
    shutItDown();
}

bool ChatServer::create(std::string ip, int port, int maxConnections, int recvBuffer)
{
    //Create the socket

    server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server == INVALID_SOCKET)
    {
        std::cout << "Socket Creation Failed: " << WSAGetLastError() << std::endl;
        return false;
    }

    //Bind the socket

    sockaddr_in service;
    service.sin_family = AF_INET;
    InetPtonA(AF_INET, ip.c_str(), &service.sin_addr.S_un); //Do not use InetPton as it defaults to InetPtonW (using widestr), instead use InetPtonA
    service.sin_port = htons(port);
    if (bind(server, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR)
    {
        std::cout << "Bind failed: " << WSAGetLastError() << std::endl;
        closesocket(server);
        return false;
    }

    //Perform a deep copy of the data
    this->ip = ip;
    this->port = port;
    this->maxConnections = maxConnections;
    this->maxAllowedChars = recvBuffer;
    return true;
}

void ChatServer::listenForClients()
{
    isListenRunning = true;
    listenThread = std::thread([this]() {
        //Listen
        if (listen(server, maxConnections) == SOCKET_ERROR)
        {
            std::cerr << "Listening Failed: " << WSAGetLastError() << std::endl;
            stopThread();
            return;
        }

        //Accept new client
        while (isListenRunning) {
            fd_set readSet;
            FD_ZERO(&readSet);
            FD_SET(server, &readSet);

            timeval timeout;
            timeout.tv_sec = 1;  // Timeout in seconds
            timeout.tv_usec = 0; // Timeout in microseconds

            int activity = select(0, &readSet, NULL, NULL, &timeout);
            if (activity == SOCKET_ERROR) {
                std::cerr << "Select failed: " << WSAGetLastError() << std::endl;
                break;
            }

            if (activity > 0 && FD_ISSET(server, &readSet)) {
                SOCKET potentialClient = accept(server, NULL, NULL);
                if (potentialClient == INVALID_SOCKET) {
                    std::cerr << "Accept Failed: " << WSAGetLastError() << std::endl;
                    continue;
                }

                // Process new connection
                int clientID = totalConnections;
                clientConnectionFlags[clientID] = std::make_pair(
                    std::make_unique<std::atomic<bool>>(true),
                    potentialClient
                );
                std::thread(&ChatServer::recvFromClient, this, clientID).detach();
                totalConnections++;
            }
            if (totalConnections == maxConnections)
                stopListening();
        }

        std::cout << "Server has stopped listening for new connections." << std::endl;

    });
}

void ChatServer::recvFromClient(int clientID)
{   
    // Retrieve socket and running flag
    auto& entry = clientConnectionFlags[clientID];
    auto& runningFlag = *entry.first; // Dereference unique_ptr to access atomic<bool>
    SOCKET client = entry.second;

    std::cout << "Thread handling client: " << clientID << " started." << std::endl;

    ChatData data;
    int bytesReceived;
    while(true)
    {
        //Store message and check for errors
        bytesReceived = recv(client, (char*)&data, sizeof(data), 0);
        if (bytesReceived == SOCKET_ERROR)
        {
            std::cerr << "Client " << clientID << " received failed: " << WSAGetLastError() << std::endl;
            break;
        }

        if (bytesReceived == 0)
        {
            std::cout << "Client " << clientID << " has disconnected" << std::endl;
            break;
        }
        std::cout << "Bytes: " << bytesReceived << std::endl;

        std::cout << "Client " << clientID << ": " << data.getMessage() << std::endl;
    }
    stopClient(clientID); //Stop thread
    closesocket(client);
    clientConnectionFlags.erase(clientID);
}

void ChatServer::stopThread()
{
    isListenRunning = false;
    //If the thread is still running, wait for it to finish before stopping
    if (listenThread.joinable()) 
        listenThread.join();
}

void ChatServer::stopListening()
{
    stopThread();
}

void ChatServer::shutItDown()
{
    stopThread(); //PROBLEMS
    for (auto& [clientID, pair] : clientConnectionFlags)
    {
        stopClient(clientID);
    }
    shutdown(server, SD_BOTH);
    closesocket(server);
    std::cout << "Server succesfully terminated." << std::endl;
}

void ChatServer::stopClient(int clientID)
{
    if (clientConnectionFlags.find(clientID) != clientConnectionFlags.end())
    {
        *clientConnectionFlags[clientID].first = false; // Set running flag to false

        // Optionally close the socket to force termination
        SOCKET clientSocket = clientConnectionFlags[clientID].second;
        shutdown(clientSocket, SD_BOTH);
        closesocket(clientSocket);

        // Remove the client from the map
        clientConnectionFlags.erase(clientID);
    }
}

void ChatServer::sendToClient(int clientID, const char message[])
{
    int byteCount;
    ChatData data;
    data.setMessage(message);
    if (clientConnectionFlags.find(clientID) != clientConnectionFlags.end())
    {
        SOCKET clientSocket = clientConnectionFlags[clientID].second;

        // Send the message
        byteCount = send(clientSocket, (char*)&data, sizeof(data), 0);
        if (byteCount == SOCKET_ERROR)
        {
            std::cerr << "Failed to send message to client " << clientID << ": " << WSAGetLastError() << std::endl;
        }
        else
        {
            std::cout << byteCount << std::endl;
            std::cout << "Message sent to client " << clientID << ": " << message << std::endl;
        }
    }
    else
    {
        std::cerr << "Client " << clientID << " not found or disconnected." << std::endl;
    }
}

void ChatServer::broadcast(const std::string& message)
{
    for (auto& [clientID, pair] : clientConnectionFlags)
    {
        SOCKET clientSocket = pair.second;

        // Send the message to the current client
        int result = send(clientSocket, message.c_str(), static_cast<int>(message.size()), 0);
        if (result == SOCKET_ERROR)
        {
            std::cerr << "Failed to send message to client " << clientID << ": " << WSAGetLastError() << std::endl;
        }
        else
        {
            std::cout << "Message broadcasted to client " << clientID << ": " << message << std::endl;
        }
    }
}



