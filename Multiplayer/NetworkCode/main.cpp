#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h> //InetPtonA function

#include <iostream>
#include <string>

#include <thread>
#include <atomic>
#include <future>
#include <condition_variable>
#include <vector>
#include <chrono> //Used to make thread wait (for non-blocking sockets)

#include "ChatObject.h"

//Required to initialize winsock2 and the sockets
namespace { //Ensures these cannot be seen outside of the file
WSAData wsaData;

//Connection information
std::string port;
std::string ip;

//Sockets
SOCKET server;
SOCKET client;
}

struct Network_Server_Locks
{
    std::mutex client_vector_mtx;
    std::mutex error_msg_buffer_mtx;
    std::mutex client_vector_capacity_mtx;
    std::condition_variable client_vector_cv; //Used for operation of listening and receiving threads
    std::mutex handling_data_mtx;
    std::condition_variable handling_data_cv;
};

struct Network_Server_Information
{
    std::vector<SOCKET> client_vector;
    std::vector<std::string> error_msg_buffer;
    std::atomic<bool> isListening;
    std::atomic<bool> isReceiving;
    std::thread serverListeningThread;
    std::thread serverReceivingThread;
};

#define MAX_CONNECTIONS 4

Network_Server_Locks networkLocks;
Network_Server_Information networkInfo;

void Network_Server_Receive();
void Network_HandleData(const ChatObject& data);
void Network_InitializeWSA();
bool Network_Server_CreateSocket();
bool Network_Server_Bind();
void Network_Server_Init();
bool Network_Server_InitThreads();
void Network_Server_Shutdown();

int main(void)
{
    Network_InitializeWSA();
    Network_Server_Init();

    //Need to remove thread here and conditional variable for operation, its stupid, just use a bool.
    std::thread run;

    std::cout << "Enter \"s\" for server or \"c\" if client: ";
    std::string code;
    std::getline(std::cin, code);
    std::cout << std::endl;
    if (code == "s") 
    {
        if (!Network_Server_CreateSocket()) return 1;
        if (!Network_Server_Bind()) return 1;
        if (!Network_Server_InitThreads()) return 1;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    Network_Server_Shutdown();

     std::cout << "Program Complete" << std::endl;
    return 0;
}

void Network_Server_Receive()
{
    ChatObject dataReceived;
    networkInfo.isReceiving = true;
    size_t client_vector_size = 0;
    SOCKET client;
    //We wait 1ms to check if more data will arrive
    fd_set data_check;
    struct timeval select_wait;
    select_wait.tv_sec = 0;
    select_wait.tv_usec = 10000; //Creates waiting
    while (networkInfo.isReceiving)
    {
        {   //Get capacity
            std::unique_lock lock(networkLocks.client_vector_mtx);
            size_t client_vector_size = networkInfo.client_vector.size();
        }
        if (client_vector_size == 0)
        {
            std::unique_lock lock(networkLocks.client_vector_capacity_mtx);
            networkLocks.client_vector_cv.wait(lock);
        } else {
            networkLocks.client_vector_cv.notify_one();
        }

        for (int i = 0; i < client_vector_size; i++)
        {
            {   //Get the size and client
                std::unique_lock lock(networkLocks.client_vector_mtx);
                size_t client_vector_size = networkInfo.client_vector.size();
                client = networkInfo.client_vector[i];
            }
            FD_ZERO(&data_check); //Clear the previous socket from the set, as we only want to check the current one
            FD_SET(client, &data_check); //Add current socket to set
            int check = select(0, &data_check, NULL, NULL, &select_wait);
            //TODO: Verify that check = 0 to continue, if not, it should say that receiving message failed cause timeout and continue to next socket
            int bytesReceived = recv(client, (char*)&dataReceived, sizeof(dataReceived), 0);
            if (bytesReceived == 0) //Client has disconnected
            {
                std::cout << "Client Disconnected" << std::endl;
                shutdown(client, SD_BOTH);
                closesocket(client);
                {   //Delete socket from client_vector
                    std::unique_lock lock1(networkLocks.client_vector_mtx);
                    networkInfo.client_vector.erase(networkInfo.client_vector.begin() + i);
                    std::lock_guard lock2(networkLocks.client_vector_capacity_mtx);
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
    networkInfo.isListening = true;
    //Set server and new client sockets to non-blocking
    u_long mode = 1; //Non-blocking mode 
    ioctlsocket(server, FIONBIO, &mode); //Set server to non-blocking
    size_t client_vector_size;
    while (networkInfo.isListening) //Continuously accept new clients
    {
        {//Verify if the server is empty
            std::unique_lock<std::mutex> lock(networkLocks.client_vector_mtx);
            client_vector_size = networkInfo.client_vector.size();
        }
        SOCKET newClient = accept(server, NULL, NULL);
        if (newClient == INVALID_SOCKET)
        {
            std::unique_lock<std::mutex> lock(networkLocks.error_msg_buffer_mtx);
            networkInfo.error_msg_buffer.push_back("Connection error has occured.");
        }
        //Accepts new client if possible
        if (client_vector_size < MAX_CONNECTIONS)
        {
            ioctlsocket(newClient, FIONBIO, &mode); //Sets newClient to be non-blocking
            std::unique_lock<std::mutex> lock(networkLocks.client_vector_mtx);
            networkInfo.client_vector.push_back(newClient);
            lock.unlock();
            networkLocks.client_vector_cv.notify_one();
        } else
        { //Lock listening until the server is non-empty or until we want to stop listening
            std::unique_lock lock(networkLocks.client_vector_capacity_mtx);
            networkLocks.client_vector_cv.wait(lock);
        }
        std::this_thread::sleep_for(std::chrono::seconds(1)); //Wait a second before checking again
    }
}

bool Network_Server_Bind()
{
    //Get usert input
    std::cout << "Enter 5-digit port number: ";
    std::getline(std::cin, port);
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
    service.sin_port = htons(std::stoi(port));
    if (bind(server, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR)
    {
        closesocket(server);
        return false;
    }
    return true;
}

bool Network_Server_CreateSocket()
{
    //Initialize TCP server
    server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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
        std::unique_lock<std::mutex> lock(networkLocks.error_msg_buffer_mtx);
        networkInfo.error_msg_buffer.push_back("WSA initialization failed.");
    }
}

void Network_Server_Init()
{
    networkInfo.isListening = false;
    networkInfo.isReceiving = false;
}

bool Network_Server_InitThreads()
{
    if (listen(server, MAX_CONNECTIONS) == SOCKET_ERROR) return false;
    networkInfo.serverListeningThread = std::thread(Network_Server_Listen);
    networkInfo.serverReceivingThread = std::thread(Network_Server_Receive);
    return true;
}

void Network_Server_Shutdown()
{
    networkInfo.isReceiving = false;
    networkInfo.isListening = false;
    /*Since both threads are used to signal the other to continue. Killing one makes the other stuck on the cv.
    Hence the need for one last notification.*/
    networkLocks.client_vector_cv.notify_all(); 

    std::cout << "Debug0 About to join Listen" << std::endl;
    networkInfo.serverListeningThread.join();
    std::cout << "Debug1 Listen joined, joining receive" << std::endl;
    networkInfo.serverReceivingThread.join();
    std::cout << "Debug2 All joined" << std::endl;
    shutdown(server, SD_BOTH);
    closesocket(server);
    server = INVALID_SOCKET;
}