#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h> //InetPtonA function

#include <iostream>
#include <string>

#include <thread>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <vector>
#include <chrono> //Used to make thread wait (for non-blocking sockets)

#include "ChatObject.h"

//Required to initialize winsock2 and the sockets
namespace { //Ensures these cannot be seen outside of the file
WSAData wsaData;

//Sockets
}

struct Network_Server_Locks
{
    std::mutex client_vector_mtx;
    std::mutex client_vector_capacity_mtx;
    std::condition_variable client_vector_cv; //Used for operation of listening and receiving threads
    std::mutex handling_data_mtx;
    std::condition_variable handling_data_cv;
    bool isServerFull;
    bool isServerEmpty;
};

struct Network_Server_Information
{
    std::vector<SOCKET> client_vector;
    std::atomic<bool> isListening;
    std::atomic<bool> isReceiving;
    std::thread serverListeningThread; //1 second wait
    std::thread serverReceivingThread; //0.01 second wait
    std::string port;
    SOCKET server;
    int MAX_CONNECTIONS;
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
    SOCKET client;
    int CLIENT_CONNECT_TIMEOUT_SEC;
};

struct Network_Client_Locks
{
    std::mutex client_mtx;
    std::mutex winsock_mtx;
    std::condition_variable send_data_cv;
};

Network_Client_Information networkClientInfo;

void Network_InitializeWSA(Network_Error_Msg_Buffer& errors);
void Network_Server_Init(int max_allowed_connections, Network_Server_Locks& networkServerLocks, Network_Server_Information& networkServerInfo);
void Network_Server_Set_Port(const std::string& port_in, Network_Server_Information& networkServerInfo);
bool Network_Server_CreateSocket(Network_Server_Information& networkServerInfo);
bool Network_Server_Bind(Network_Server_Information& networkServerInfo);
bool Network_Server_LoopbackBind(Network_Server_Information& networkServerInfo);
bool Network_Server_InitThreads(Network_Error_Msg_Buffer& errors, Network_Server_Locks& networkServerLocks, Network_Server_Information& networkServerInfo);
void Network_Server_Receive(Network_Error_Msg_Buffer& errors, Network_Server_Locks& networkServerLocks, Network_Server_Information& networkServerInfo); //Thread
void Network_Server_Listen(Network_Error_Msg_Buffer& errors, Network_Server_Locks& networkServerLocks, Network_Server_Information& networkServerInfo); //Thread
void Network_Server_Updates(Network_Server_Locks& networkServerLocks, Network_Server_Information& networkServerInfo);
void Network_Server_Shutdown(Network_Server_Locks& networkServerLocks, Network_Server_Information& networkServerInfo);
void Network_Client_Init(int client_connect_seconds_timeout);
void Network_Client_Set_Ip(const std::string& ip_in);
void Network_Client_Set_Port(const std::string& port_in);
bool Network_Client_CreateSocket(Network_Client_Locks& networkClientLocks);
bool Network_Client_ConnectToServer(Network_Error_Msg_Buffer& errors, Network_Client_Locks& networkClientLocks);
bool Network_Client_LoopbackConnect(Network_Error_Msg_Buffer& errors, Network_Client_Locks& networkClientLocks);
void Network_Client_InitThreads(Network_Error_Msg_Buffer& errors, Network_Client_Locks& networkClientLocks);
void Network_Client_SendToServer(Network_Error_Msg_Buffer& errors, Network_Client_Locks& networkClientLocks); //Thread
void Network_Client_Updates(Network_Client_Locks& networkClientLocks);
void Network_Client_Shutdown(Network_Client_Locks& networkClientLocks);
void Network_HandleData(const ChatObject& data); //Handle data

int main(void)
{
    Network_Error_Msg_Buffer gen_err;
    Network_Server_Locks server_locks;
    Network_Server_Information server_info;
    Network_Client_Locks client_locks;
    Network_InitializeWSA(gen_err);
    std::cout << "Enter \"s\" for server or \"c\" if client: ";
    std::string code;
    std::getline(std::cin, code);
    std::cout << std::endl;
    if (code == "s") 
    {
        std::cout << "Server" << std::endl;
        Network_Server_Init(4, server_locks, server_info);
        if (!Network_Server_CreateSocket(server_info)) return 1;
        if (!Network_Server_LoopbackBind(server_info)) return 1;
        if (!Network_Server_InitThreads(gen_err, server_locks, server_info)) return 1;
        for (;;)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            Network_Server_Updates(server_locks, server_info);
            std::unique_lock<std::mutex> lockA(gen_err.msg_buffer_mtx);
            if (gen_err.msg_buffer.size() > 0)
            {
                std::cout << "Cur ErrorVec Size: " << gen_err.msg_buffer.size() << "  Error msg: " << gen_err.msg_buffer.back() << std::endl;
            } 
            std::unique_lock<std::mutex> lockB(server_locks.client_vector_capacity_mtx);
            std::cout << "Vector Size: " << server_info.client_vector.size() << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        std::cout << "STATUS: Updating" << std::endl;
        Network_Server_Updates(server_locks, server_info);
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        Network_Server_Shutdown(server_locks, server_info);
    }
    if (code == "c")
    {
        std::cout << "Client" << std::endl;
        Network_Client_Init(5);
        if (!Network_Client_CreateSocket(client_locks)) return 1;
        if (!Network_Client_LoopbackConnect(gen_err, client_locks)) return 1;
        Network_Client_InitThreads(gen_err, client_locks);
        std::string msg;
        std::cout << "Enter message: ";
        std::getline(std::cin, msg);
        networkClientInfo.dataToSend.setMessage(msg);
        Network_Client_Updates(client_locks);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        Network_Client_Shutdown(client_locks);
    }
    std::cout << "Program Complete" << std::endl;
    return 0;
}

void Network_Server_Receive(Network_Error_Msg_Buffer& errors, Network_Server_Locks& networkServerLocks, Network_Server_Information& networkServerInfo) //This function executes a LOT of times per second, may put cap in future (depending on cpu obliteration)
{
    ChatObject dataReceived;
    networkServerInfo.isReceiving = true;
    size_t client_vector_size = 0;
    SOCKET clientSoc;
    //We wait 1ms to check if more data will arrive
    fd_set data_check;
    struct timeval select_wait;
    select_wait.tv_sec = 0;
    select_wait.tv_usec = 10000; //Creates waiting (10 ms)
    while (networkServerInfo.isReceiving)
    {
        {   //Get capacity
            std::unique_lock lock(networkServerLocks.client_vector_mtx);
            client_vector_size = networkServerInfo.client_vector.size();
        }
        if (client_vector_size == 0)
        { //Halt thread if the server is empty
            std::unique_lock lock(networkServerLocks.client_vector_capacity_mtx);
            networkServerLocks.isServerEmpty = true;
            networkServerLocks.client_vector_cv.wait(lock, [&networkServerLocks]{return !networkServerLocks.isServerEmpty;}); //Capture by reference
        } 
        for (int i = 0; i < client_vector_size; i++)
        {
            {   //Get the size and client
                std::unique_lock lock(networkServerLocks.client_vector_mtx);
                client_vector_size = networkServerInfo.client_vector.size();
                clientSoc = networkServerInfo.client_vector[i];
            }
            FD_ZERO(&data_check); //Clear the previous socket from the set, as we only want to check the current one
            FD_SET(clientSoc, &data_check); //Add current socket to set
            int check = select(0, &data_check, NULL, NULL, &select_wait);
            //TODO: Verify that check = 0 to continue, if not, it should say that receiving message failed cause timeout and continue to next socket
            //if (check == 0) continue;
            int bytesReceived = recv(clientSoc, (char*)&dataReceived, sizeof(dataReceived), 0);
            if (bytesReceived == 0) //Client has disconnected
            {
                std::cout << "Client Disconnected" << std::endl;
                shutdown(clientSoc, SD_BOTH);
                closesocket(clientSoc);
                std::unique_lock lock1(networkServerLocks.client_vector_mtx);
                std::lock_guard lock2(networkServerLocks.client_vector_capacity_mtx);
                networkServerInfo.client_vector.erase(networkServerInfo.client_vector.begin() + i); //Delete socket from client_vector
                if (networkServerInfo.client_vector.size() == 0) break;
                i--;
                client_vector_size--; //Prevent WSAENOTSOCK (10038) error
                continue;
            }
            if (bytesReceived == SOCKET_ERROR) //Receive failed for some reason
            {
                std::unique_lock<std::mutex> lock(errors.msg_buffer_mtx);
                int err = WSAGetLastError();
                if (err != WSAEWOULDBLOCK && err != 0) //Log the error if it is not WSAEWOULDBLOCK
                    errors.msg_buffer.push_back("Connection error has occured: " + std::to_string(err));
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

void Network_Server_Listen(Network_Error_Msg_Buffer& errors, Network_Server_Locks& networkServerLocks, Network_Server_Information& networkServerInfo) //Error occurs here for CV
{
    networkServerInfo.isListening = true;
    //Set server and new client sockets to non-blocking
    u_long mode = 1; //Non-blocking mode 
    ioctlsocket(networkServerInfo.server, FIONBIO, &mode); //Set server to non-blocking
    size_t client_vector_size;
    while (networkServerInfo.isListening) //Continuously accept new clients
    {
        std::this_thread::sleep_for(std::chrono::seconds(1)); //Wait a second before checking again (bot spam protection)
        {//Verify if the server is empty
            std::unique_lock<std::mutex> lock(networkServerLocks.client_vector_mtx);
            client_vector_size = networkServerInfo.client_vector.size();
        }
        SOCKET newClient = accept(networkServerInfo.server, NULL, NULL);
        if (newClient == INVALID_SOCKET)
        {
            std::unique_lock<std::mutex> lock(errors.msg_buffer_mtx);
            int err = WSAGetLastError();
            if (err != WSAEWOULDBLOCK && err != 0) //Take note of the error if it is not a would block call
            {
                errors.msg_buffer.push_back("Listen error has occured: " + std::to_string(err));
                continue;
            }
            continue; 
        }
        //Accepts new client if possible
        if (client_vector_size < networkServerInfo.MAX_CONNECTIONS)
        {
            ioctlsocket(newClient, FIONBIO, &mode); //Sets newClient to be non-blocking
            std::unique_lock<std::mutex> lock(networkServerLocks.client_vector_mtx);
            networkServerInfo.client_vector.push_back(newClient);
        } else
        { //Lock listening until the server is non-empty
            std::unique_lock lock(networkServerLocks.client_vector_capacity_mtx);
            networkServerLocks.isServerFull = true;
            networkServerLocks.client_vector_cv.wait(lock, [&networkServerLocks]{return !networkServerLocks.isServerFull;}); //capture by reference
        }
    }
}

bool Network_Server_Bind(Network_Server_Information& networkServerInfo)
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
    if (bind(networkServerInfo.server, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR)
    {
        closesocket(networkServerInfo.server);
        return false;
    }
    return true;
}

bool Network_Server_CreateSocket(Network_Server_Information& networkServerInfo)
{
    networkServerInfo.server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //Initialize TCP server
    if (networkServerInfo.server == INVALID_SOCKET) return false;
    return true;
}

void Network_InitializeWSA(Network_Error_Msg_Buffer& errors)
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

void Network_Server_Init(int max_allowed_connections, Network_Server_Locks& networkServerLocks, Network_Server_Information& networkServerInfo)
{
    networkServerInfo.isListening = false;
    networkServerInfo.isReceiving = false;
    networkServerLocks.isServerEmpty = true;
    networkServerLocks.isServerFull = false;
    networkServerInfo.MAX_CONNECTIONS = max_allowed_connections;
}

bool Network_Server_InitThreads(Network_Error_Msg_Buffer& errors, Network_Server_Locks& networkServerLocks, Network_Server_Information& networkServerInfo)
{
    if (listen(networkServerInfo.server, networkServerInfo.MAX_CONNECTIONS) == SOCKET_ERROR) return false;
    networkServerInfo.serverListeningThread = std::thread(Network_Server_Listen, std::ref(errors), std::ref(networkServerLocks), std::ref(networkServerInfo));
    networkServerInfo.serverReceivingThread = std::thread(Network_Server_Receive, std::ref(errors), std::ref(networkServerLocks), std::ref(networkServerInfo));
    return true;
}

void Network_Server_Shutdown(Network_Server_Locks& networkServerLocks, Network_Server_Information& networkServerInfo)
{
    networkServerInfo.isReceiving = false;
    networkServerInfo.isListening = false;
    /*Since both threads are used to signal the other to continue. Killing one makes the other stuck on the cv.
    Hence the need for one last notification.*/
    networkServerLocks.client_vector_cv.notify_all(); 
    networkServerInfo.serverListeningThread.join();
    networkServerInfo.serverReceivingThread.join();
    shutdown(networkServerInfo.server, SD_BOTH);
    closesocket(networkServerInfo.server);
    networkServerInfo.server = INVALID_SOCKET;
}

bool Network_Client_CreateSocket(Network_Client_Locks& networkClientLocks)
{
    networkClientInfo.client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //Initialize TCP server
    if (networkClientInfo.client == INVALID_SOCKET) return false;
    u_long mode = 1;
    {
        std::unique_lock<std::mutex> lock(networkClientLocks.client_mtx);
        ioctlsocket(networkClientInfo.client, FIONBIO, &mode); //Set to non-blocking
    }
    return true;
}

bool Network_Client_ConnectToServer(Network_Error_Msg_Buffer& errors, Network_Client_Locks& networkClientLocks) //To be run on another thread while main thread uses loading screen
{
    sockaddr_in clientService; //Connection information
    {
        std::unique_lock<std::mutex> lock(networkClientLocks.client_mtx);
        clientService.sin_family = AF_INET;
        InetPtonA(AF_INET, networkClientInfo.ip.c_str(), &clientService.sin_addr.S_un); //Turn ipv4/ipv6 into standard text representation
        clientService.sin_port = htons(std::stoi(networkClientInfo.port)); //Convert port to network byte order (big-endian)
    }
    fd_set client_check;
    FD_ZERO(&client_check);
    FD_SET(networkClientInfo.client, &client_check); //Add current socket to set
    struct timeval select_wait;
    select_wait.tv_sec = networkClientInfo.CLIENT_CONNECT_TIMEOUT_SEC;
    select_wait.tv_usec = 0;
    int connectRes;
    {
        std::unique_lock<std::mutex> lock(networkClientLocks.winsock_mtx);
        int connectRes = connect(networkClientInfo.client, (SOCKADDR*)&clientService, sizeof(clientService)); //Begin connection attempt
        if (connectRes == 0) return true; //Connection successfull
        connectRes = WSAGetLastError(); //Check error code to determine if it should block
        if (connectRes != WSAEWOULDBLOCK)
        {
            std::unique_lock<std::mutex> lock(errors.msg_buffer_mtx);
            errors.msg_buffer.push_back("Connection did not block");
            return false;
        }
        connectRes = select(networkClientInfo.client + 1, NULL, &client_check, NULL, &select_wait); //Wait 3 seconds to see if connection is successfull
        if (connectRes <= 0) //Timeout is 0 and anything < 0 means an error with select has occurred
        {
            std::unique_lock<std::mutex> lock(errors.msg_buffer_mtx);
            connectRes == 0 ? errors.msg_buffer.push_back("Connection Timeout") : errors.msg_buffer.push_back("Select error");
            return false;
        } 
        socklen_t len = sizeof(connectRes);
        getsockopt(networkClientInfo.client, SOL_SOCKET, SO_ERROR, (char*)&connectRes, &len);
        if (connectRes != 0) //Connection failed
        {
            std::unique_lock<std::mutex> lock(errors.msg_buffer_mtx);
            errors.msg_buffer.push_back("Connection Failed");
            return false; 
        }
        //If we get here, then the connection was successfull, and it is linked with the server        
    }
    return true;
}

void Network_Client_SendToServer(Network_Error_Msg_Buffer& errors, Network_Client_Locks& networkClientLocks)
{
    int byteCount;
    u_long mode = 1; 
    networkClientInfo.isSending = true;
    while (1)
    {
        std::unique_lock<std::mutex> lock(networkClientLocks.client_mtx);
        networkClientLocks.send_data_cv.wait(lock);
        if (!networkClientInfo.isSending) break; //End thread
        byteCount = send(networkClientInfo.client, (char*)&networkClientInfo.dataToSend, sizeof(networkClientInfo.dataToSend), 0);
        if (byteCount == SOCKET_ERROR)
        {
            std::unique_lock<std::mutex> lock(errors.msg_buffer_mtx);
            errors.msg_buffer.push_back("Message send failed");
        }
        networkClientInfo.isMsgReady = false; //Redundant for now
    }
}

void Network_Client_Init(int client_connect_seconds_timeout)
{
    networkClientInfo.isSending = false;
    networkClientInfo.isMsgReady = false;
    networkClientInfo.CLIENT_CONNECT_TIMEOUT_SEC = client_connect_seconds_timeout;
}

void Network_Client_InitThreads(Network_Error_Msg_Buffer& errors, Network_Client_Locks& networkClientLocks)
{
    networkClientInfo.clientSendThread = std::thread(Network_Client_SendToServer, std::ref(errors), std::ref(networkClientLocks));
}

void Network_Client_Shutdown(Network_Client_Locks& networkClientLocks)
{
    networkClientInfo.isSending = false;
    networkClientLocks.send_data_cv.notify_one(); //Breaks out of the deadlock, but skips over resending data.
    networkClientInfo.clientSendThread.join();
    closesocket(networkClientInfo.client); //server handles shutdown()
    networkClientInfo.client = INVALID_SOCKET;
}

void Network_Server_Set_Port(const std::string& port_in, Network_Server_Information& networkServerInfo)
{
    networkServerInfo.port = port_in;
}

void Network_Client_Set_Ip(const std::string& ip_in)
{
    networkClientInfo.ip = ip_in;
}

void Network_Client_Set_Port(const std::string& port_in)
{
    networkClientInfo.port = port_in;
}

bool Network_Client_LoopbackConnect(Network_Error_Msg_Buffer& errors, Network_Client_Locks& networkClientLocks)
{
    networkClientInfo.port = "55555";
    networkClientInfo.ip = "127.0.0.1";
    return Network_Client_ConnectToServer(errors, networkClientLocks);
}

void Network_Server_Updates(Network_Server_Locks& networkServerLocks, Network_Server_Information& networkServerInfo)
{
    {
        std::scoped_lock lock(networkServerLocks.client_vector_mtx, networkServerLocks.client_vector_capacity_mtx);
        size_t client_vector_size = networkServerInfo.client_vector.size();
        (client_vector_size == 0) ? networkServerLocks.isServerEmpty = true : networkServerLocks.isServerEmpty = false;
        (client_vector_size >= networkServerInfo.MAX_CONNECTIONS) ? networkServerLocks.isServerFull = true : networkServerLocks.isServerFull = false;
    }
    networkServerLocks.client_vector_cv.notify_all();
}

void Network_Client_Updates(Network_Client_Locks& networkClientLocks)
{
    std::lock_guard<std::mutex> lock(networkClientLocks.client_mtx);
    networkClientLocks.send_data_cv.notify_all();
}

bool Network_Server_LoopbackBind(Network_Server_Information& networkServerInfo)
{
    std::string ip = "127.0.0.1";
    networkServerInfo.port = "55555";
    //Bind the socket
    sockaddr_in service;
    service.sin_family = AF_INET;
    InetPtonA(AF_INET, ip.c_str(), &service.sin_addr.S_un);
    service.sin_port = htons(std::stoi(networkServerInfo.port));
    if (bind(networkServerInfo.server, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR)
    {
        closesocket(networkServerInfo.server);
        return false;
    }
    return true;
}