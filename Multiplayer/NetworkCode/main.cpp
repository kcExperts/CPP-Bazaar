#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h> //InetPtonA function

#include <iostream>
#include <string>

#include <thread>
#include <atomic>
#include <condition_variable>
#include <vector>
#include <chrono> //Used to make thread wait (for non-blocking sockets)

#include "ChatObject.h"

//Required to initialize winsock2 and the sockets
namespace { //Ensures these cannot be seen outside of the file
WSAData wsaData;

//Sockets
SOCKET server;
SOCKET client;
}

struct Network_Server_Locks
{
    std::mutex client_vector_mtx;
    std::mutex client_vector_capacity_mtx;
    std::condition_variable client_vector_cv; //Used for operation of listening and receiving threads
    std::mutex handling_data_mtx;
    std::condition_variable handling_data_cv;
};

struct Network_Server_Information
{
    std::vector<SOCKET> client_vector;
    std::atomic<bool> isListening;
    std::atomic<bool> isReceiving;
    std::thread serverListeningThread;
    std::thread serverReceivingThread;
    std::string port;
};

struct Network_Error_Msg_Buffer
{
    std::vector<std::string> msg_buffer;
     std::mutex msg_buffer_mtx;
};

struct Network_Client_Information
{
    std::string port;
    std::string ip;
    std::atomic<bool> isSending;
    ChatObject dataToSend;
    std::thread clientSendThread;
    bool isMsgReady;
};

struct Network_Client_Locks
{
    std::mutex client_mtx;
    std::mutex winsock_mtx;
    std::condition_variable send_data_cv;
};

#define MAX_CONNECTIONS 4
#define CLIENT_CONNECT_TIMEOUT_SEC 1

Network_Server_Locks networkServerLocks;
Network_Server_Information networkServerInfo;
Network_Client_Information networkClientInfo;
Network_Client_Locks networkClientLocks;
Network_Error_Msg_Buffer errors;

void Network_Server_Receive();
void Network_HandleData(const ChatObject& data);
void Network_InitializeWSA();
bool Network_Server_CreateSocket();
bool Network_Server_Bind();
void Network_Server_Init();
bool Network_Server_InitThreads();
void Network_Server_Shutdown();
bool Network_Client_CreateSocket();
bool Network_Client_ConnectToServer();
void Network_Client_SendToServer();
void Network_Client_Init();
void Network_Client_InitThreads();
void Network_Client_Shutdown();

bool Network_Server_TestBind()
{
    std::string ip = "127.0.0.1";
    networkServerInfo.port = "55555";
    //Bind the socket
    sockaddr_in service;
    service.sin_family = AF_INET;
    InetPtonA(AF_INET, ip.c_str(), &service.sin_addr.S_un);
    service.sin_port = htons(std::stoi(networkServerInfo.port));
    if (bind(server, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR)
    {
        closesocket(server);
        return false;
    }
    return true;
}

bool Network_Client_ConnectToServerTest() //To be run on another thread while main thread uses loading screen
{
    u_long mode = 1; 
    ioctlsocket(client, FIONBIO, &mode); //Temporarily set socket to non-blocking
    sockaddr_in clientService; //Connection information
    //{
        //std::unique_lock<std::mutex> lock(networkClientLocks.client_mtx);
        clientService.sin_family = AF_INET;
        InetPtonA(AF_INET, networkClientInfo.ip.c_str(), &clientService.sin_addr.S_un); //Turn ipv4/ipv6 into standard text representation
        clientService.sin_port = htons(std::stoi(networkClientInfo.port)); //Convert port to network byte order (big-endian)
    //}
    fd_set client_check;
    FD_ZERO(&client_check);
    FD_SET(client, &client_check); //Add current socket to set
    struct timeval select_wait;
    select_wait.tv_sec = CLIENT_CONNECT_TIMEOUT_SEC;
    select_wait.tv_usec = 0;
    int connectRes;
    {
        //std::unique_lock<std::mutex> lock(networkClientLocks.winsock_mtx); //Prevents message from being sent
        connectRes = connect(client, (SOCKADDR*)&clientService, sizeof(clientService)); //Begin connection attempt
        if (connectRes == 0) return true; //Connection successfull
        connectRes = WSAGetLastError(); //Check error code to determine if it should block
        connectRes = select(client + 1, NULL, &client_check, NULL, &select_wait); //Wait 3 seconds to see if connection is successfull
        socklen_t len = sizeof(connectRes);
        getsockopt(client, SOL_SOCKET, SO_ERROR, (char*)&connectRes, &len);       
    }
    mode = 0; 
    ioctlsocket(client, FIONBIO, &mode); //Set back to blocking
    return true;
}

void Network_Client_SendToServerTest()
{
    int byteCount;
    u_long mode; 
    networkClientInfo.isSending = true;
    while (networkClientInfo.isSending)
    {
        mode = 1;
        {
            std::unique_lock<std::mutex> lock(networkClientLocks.client_mtx);
            networkClientLocks.send_data_cv.wait(lock);
        }
        std::cout << "Message Send" << std::endl;
    }
}

void Network_Client_InitThreadsTest()
{
    networkClientInfo.clientSendThread = std::thread(Network_Client_SendToServerTest);
}

int main(void)
{
    Network_InitializeWSA();
    std::cout << "Enter \"s\" for server or \"c\" if client: ";
    std::string code;
    std::getline(std::cin, code);
    std::cout << std::endl;
    if (code == "s") 
    {
        std::cout << "Server" << std::endl;
        Network_Server_Init();
        if (!Network_Server_CreateSocket()) return 1;
        if (!Network_Server_TestBind()) return 1;
        if (!Network_Server_InitThreads()) return 1;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        Network_Server_Shutdown();
    }
    if (code == "c")
    {
        std::cout << "Client" << std::endl;
        Network_Client_Init();
        if (!Network_Client_CreateSocket())
        {
            std::cout << "Socket Creation Failed" << std::endl;
            return 1;
        }
        networkClientInfo.port = "55555";
        networkClientInfo.ip = "127.0.0.1";
        if (!Network_Client_ConnectToServerTest())
        {
            std::cout << "Server Connection Failed" << std::endl;
            std::unique_lock<std::mutex> lock(errors.msg_buffer_mtx);
            std::cout << "ERROR: " << errors.msg_buffer.back() << std::endl;
            return 1;
        }
        Network_Client_InitThreadsTest();
        std::string msg;
        std::cout << "Enter message: ";
        std::getline(std::cin, msg);
        networkClientInfo.dataToSend.setMessage(msg);
        networkClientLocks.send_data_cv.notify_one();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        Network_Client_Shutdown();
    }
    // std::cout << "Program Complete" << std::endl;
    printf("\nProgram Complete\n");
    return 0;
}

void Network_Server_Receive()
{
    ChatObject dataReceived;
    networkServerInfo.isReceiving = true;
    size_t client_vector_size = 0;
    SOCKET clientSoc;
    //We wait 1ms to check if more data will arrive
    fd_set data_check;
    struct timeval select_wait;
    select_wait.tv_sec = 0;
    select_wait.tv_usec = 10000; //Creates waiting
    while (networkServerInfo.isReceiving)
    {
        {   //Get capacity
            std::unique_lock lock(networkServerLocks.client_vector_mtx);
            size_t client_vector_size = networkServerInfo.client_vector.size();
        }
        if (client_vector_size == 0)
        {
            std::unique_lock lock(networkServerLocks.client_vector_capacity_mtx);
            networkServerLocks.client_vector_cv.wait(lock);
        } else {
            networkServerLocks.client_vector_cv.notify_one();
        }

        for (int i = 0; i < client_vector_size; i++)
        {
            {   //Get the size and client
                std::unique_lock lock(networkServerLocks.client_vector_mtx);
                size_t client_vector_size = networkServerInfo.client_vector.size();
                clientSoc = networkServerInfo.client_vector[i];
            }
            FD_ZERO(&data_check); //Clear the previous socket from the set, as we only want to check the current one
            FD_SET(clientSoc, &data_check); //Add current socket to set
            int check = select(0, &data_check, NULL, NULL, &select_wait);
            //TODO: Verify that check = 0 to continue, if not, it should say that receiving message failed cause timeout and continue to next socket
            int bytesReceived = recv(clientSoc, (char*)&dataReceived, sizeof(dataReceived), 0);
            if (bytesReceived == 0) //Client has disconnected
            {
                std::cout << "Client Disconnected" << std::endl;
                shutdown(clientSoc, SD_BOTH);
                closesocket(clientSoc);
                {   //Delete socket from client_vector
                    std::unique_lock lock1(networkServerLocks.client_vector_mtx);
                    networkServerInfo.client_vector.erase(networkServerInfo.client_vector.begin() + i);
                    std::lock_guard lock2(networkServerLocks.client_vector_capacity_mtx);
                    i--;
                }
                continue;
            }
            if (bytesReceived == SOCKET_ERROR) //Receive failed for some reason
            {
                std::cout << "CLIENT: RECEIVED FAILED" << std::endl;
                continue;
            }
            Network_HandleData(dataReceived); //Process data
        }
    }
}

void Network_HandleData(const ChatObject& data)
{
    std::cout << data.getMessage() << std::endl;
}

void Network_Server_Listen() //Error occurs here for CV
{
    networkServerInfo.isListening = true;
    //Set server and new client sockets to non-blocking
    u_long mode = 1; //Non-blocking mode 
    ioctlsocket(server, FIONBIO, &mode); //Set server to non-blocking
    size_t client_vector_size;
    while (networkServerInfo.isListening) //Continuously accept new clients
    {
        {//Verify if the server is empty
            std::unique_lock<std::mutex> lock(networkServerLocks.client_vector_mtx);
            client_vector_size = networkServerInfo.client_vector.size();
        }
        SOCKET newClient = accept(server, NULL, NULL);
        if (newClient == INVALID_SOCKET)
        {
            std::unique_lock<std::mutex> lock(errors.msg_buffer_mtx);
            errors.msg_buffer.push_back("Connection error has occured.");
        }
        //Accepts new client if possible
        if (client_vector_size < MAX_CONNECTIONS)
        {
            ioctlsocket(newClient, FIONBIO, &mode); //Sets newClient to be non-blocking
            std::unique_lock<std::mutex> lock(networkServerLocks.client_vector_mtx);
            networkServerInfo.client_vector.push_back(newClient);
            lock.unlock();
            networkServerLocks.client_vector_cv.notify_one();
        } else
        { //Lock listening until the server is non-empty or until we want to stop listening
            std::unique_lock lock(networkServerLocks.client_vector_capacity_mtx);
            networkServerLocks.client_vector_cv.wait(lock);
        }
        std::this_thread::sleep_for(std::chrono::seconds(1)); //Wait a second before checking again (bot spam protection)
    }
}

bool Network_Server_Bind()
{
    //Get usert input
    std::string ip;
    std::cout << "Enter 5-digit port number: ";
    std::getline(std::cin, networkServerInfo.port);
    std::cout << std::endl;
    std::cout << "Enter 0 for Port forward or nothing for feedback Ip: ";
    std::string detection;
    std::getline(std::cin, detection);
    if (detection == "") ip = "127.0.0.1";
    if (detection == "0") ip = "0.0.0.0";
    //Bind the socket
    sockaddr_in service;
    service.sin_family = AF_INET;
    InetPtonA(AF_INET, ip.c_str(), &service.sin_addr.S_un);
    service.sin_port = htons(std::stoi(networkServerInfo.port));
    if (bind(server, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR)
    {
        closesocket(server);
        return false;
    }
    return true;
}

bool Network_Server_CreateSocket()
{
    server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //Initialize TCP server
    if (server == INVALID_SOCKET) return false;
    return true;
}

void Network_InitializeWSA()
{
    int wsaerr;
    //Desire version 2.2 of winsock2 (most recent as of this code)
    WORD wVersionRequested = MAKEWORD(2,2); 
    wsaerr = WSAStartup(wVersionRequested, &wsaData);
    if (wsaerr != 0)
    {
        std::unique_lock<std::mutex> lock(errors.msg_buffer_mtx);
        errors.msg_buffer.push_back("WSA initialization failed.");
    }
}

void Network_Server_Init()
{
    client = INVALID_SOCKET;
    networkServerInfo.isListening = false;
    networkServerInfo.isReceiving = false;
}

bool Network_Server_InitThreads()
{
    if (listen(server, MAX_CONNECTIONS) == SOCKET_ERROR) return false;
    networkServerInfo.serverListeningThread = std::thread(Network_Server_Listen);
    networkServerInfo.serverReceivingThread = std::thread(Network_Server_Receive);
    return true;
}

void Network_Server_Shutdown()
{
    networkServerInfo.isReceiving = false;
    networkServerInfo.isListening = false;
    /*Since both threads are used to signal the other to continue. Killing one makes the other stuck on the cv.
    Hence the need for one last notification.*/
    networkServerLocks.client_vector_cv.notify_all(); 
    networkServerInfo.serverListeningThread.join();
    networkServerInfo.serverReceivingThread.join();
    shutdown(server, SD_BOTH);
    closesocket(server);
    server = INVALID_SOCKET;
}

bool Network_Client_CreateSocket()
{
    client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //Initialize TCP server
    if (client == INVALID_SOCKET) return false;
    return true;
}

bool Network_Client_ConnectToServer() //To be run on another thread while main thread uses loading screen
{
    u_long mode = 1; 
    ioctlsocket(client, FIONBIO, &mode); //Temporarily set socket to non-blocking
    sockaddr_in clientService; //Connection information
    {
        std::unique_lock<std::mutex> lock(networkClientLocks.client_mtx);
        clientService.sin_family = AF_INET;
        InetPtonA(AF_INET, networkClientInfo.ip.c_str(), &clientService.sin_addr.S_un); //Turn ipv4/ipv6 into standard text representation
        clientService.sin_port = htons(std::stoi(networkClientInfo.port)); //Convert port to network byte order (big-endian)
    }
    fd_set client_check;
    FD_ZERO(&client_check);
    FD_SET(client, &client_check); //Add current socket to set
    struct timeval select_wait;
    select_wait.tv_sec = CLIENT_CONNECT_TIMEOUT_SEC;
    select_wait.tv_usec = 0;
    int connectRes;
    {
        std::unique_lock<std::mutex> lock(networkClientLocks.winsock_mtx);
        int connectRes = connect(client, (SOCKADDR*)&clientService, sizeof(clientService)); //Begin connection attempt
        if (connectRes == 0) return true; //Connection successfull
        connectRes = WSAGetLastError(); //Check error code to determine if it should block
        if (connectRes != WSAEWOULDBLOCK)
        {
            std::unique_lock<std::mutex> lock(errors.msg_buffer_mtx);
            errors.msg_buffer.push_back("Connection did not block");
            return false;
        }
        connectRes = select(client + 1, NULL, &client_check, NULL, &select_wait); //Wait 3 seconds to see if connection is successfull
        if (connectRes <= 0) //Timeout is 0 and anything < 0 means an error with select has occurred
        {
            std::unique_lock<std::mutex> lock(errors.msg_buffer_mtx);
            connectRes == 0 ? errors.msg_buffer.push_back("Connection Timeout") : errors.msg_buffer.push_back("Select error");
            return false;
        } 
        socklen_t len = sizeof(connectRes);
        getsockopt(client, SOL_SOCKET, SO_ERROR, (char*)&connectRes, &len);
        if (connectRes != 0) //Connection failed
        {
            std::unique_lock<std::mutex> lock(errors.msg_buffer_mtx);
            errors.msg_buffer.push_back("Connection Failed");
            return false; 
        }
        //If we get here, then the connection was successfull, and it is linked with the server        
    }
    mode = 0; 
    ioctlsocket(client, FIONBIO, &mode); //Set back to blocking
    return true;
}

void Network_Client_SendToServer()
{
    int byteCount;
    u_long mode; 
    networkClientInfo.isSending = true;
    while (networkClientInfo.isSending)
    {
        mode = 1;
        {
            std::unique_lock<std::mutex> lock(networkClientLocks.client_mtx);
            networkClientLocks.send_data_cv.wait(lock);
            ioctlsocket(client, FIONBIO, &mode); //Temporarily set socket to non-blocking
            byteCount = send(client, (char*)&networkClientInfo.dataToSend, sizeof(networkClientInfo.dataToSend), 0);
        }
        if (byteCount == SOCKET_ERROR)
        {
            std::unique_lock<std::mutex> lock(errors.msg_buffer_mtx);
            errors.msg_buffer.push_back("Message send failed");
        }
        mode = 0;
        ioctlsocket(client, FIONBIO, &mode); //Reset back to blocking
        {
            std::unique_lock<std::mutex> lock(networkClientLocks.client_mtx);
            networkClientInfo.isMsgReady = false;
        }
    }
}

void Network_Client_Init()
{
    networkClientInfo.isSending = false;
    networkClientInfo.isMsgReady = false;
}

void Network_Client_InitThreads()
{
    networkClientInfo.clientSendThread = std::thread(Network_Client_SendToServer);
}

void Network_Client_Shutdown()
{
    networkClientInfo.isSending = false;
    networkClientLocks.send_data_cv.notify_one();

    std::cout << "Attempting to close thread" << std::endl;
    networkClientInfo.clientSendThread.join();
    std::cout << "Thread joined" << std::endl;
}