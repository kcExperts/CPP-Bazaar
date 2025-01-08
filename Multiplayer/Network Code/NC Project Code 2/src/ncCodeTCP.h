#ifndef NCCODETCP_H
#define NCCODETCP_H

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <string>
#include <atomic>
#include <algorithm>
#include <vector>
#include <iostream>
#include <thread>

enum Network_Error_Types
{
    None,
    Socket_Creation_Failed,
    Socket_Bind_Failed,
    Listen_Error,
    Receive_Select_Timeout,
    Client_Disconnected,
    Unknown_Receive_Error_Occured,
    Standard_Receive_Error_Occured,
    Standard_Broadcast_Error_Occured
};

struct Network_Error
{
    std::mutex mtx;
    Network_Error_Types type;
};

struct Socket_WL
{
    std::mutex mtx;
    SOCKET socket;
};

namespace {
    WSAData NETWORK_WSADATA;
    #define NETWORK_TCP_DATA_OBJ_MAX_ARR_SIZE 1000
    #define NETWORK_NOBODY -1
}

std::mutex Network_TCP_Data_Obj_mtx;

class Network_TCP_Data_Obj
{
    private:
        int timesToSendData;
        char array[NETWORK_TCP_DATA_OBJ_MAX_ARR_SIZE];
        int array_size;
    public:
        Network_TCP_Data_Obj();
        char getChar(int i) const;
        std::string getString() const;
        bool setMessage(const std::string& str);
};

struct Network_Data_Send_Obj
{
    Network_TCP_Data_Obj info;
    std::mutex mtx;
};

//Intialize WSA
bool Network_InitializeWSA();

//Server --
class Server
{
    public:
        //Function to handle incoming data. Set before calling initialize.
        std::function<void(const int& i, const Network_TCP_Data_Obj& data)> data_Handler_Func;
        std::function<void(const int& i)> client_disconnected;
        std::function<void(const int& i)> client_connection_error;
        std::function<void(const int& i)> broadcast_to_client_error;
        //Creates, binds the socket, giving loopback if needed
        bool initialize(const std::string& port_in);
        void close();
        Server(u_short max_connections);
    private:
        std::thread logic;
        void start(); //Runs on logic thread
        Network_Data_Send_Obj server_data;
        size_t max_connections;
        Socket_WL server;
        Network_Error err;
        std::vector<SOCKET> client_vector;
        std::mutex client_vector_mtx;
        //Start function modifiers
        std::atomic<bool> dataReadyToSend;
        std::atomic<bool> canListen;
        std::atomic<bool> toClose;

        //Retrieves the last known error message given by the program
        Network_Error getLastErrorMsg();
        size_t getCurrentServerSize();
        void Broadcast(int exception, const Network_Data_Send_Obj& info);
};

class Client
{
    public:
        std::mutex connect_mtx;
        //Function to handle incoming data. Set before calling initialize.
        std::function<void(const Network_TCP_Data_Obj& data)> data_Handler_Func;
        bool initialize(const std::string& ip_in, const std::string& port_in);
        void start();
        void close();
    private:
        Network_Error err;
        //Retrieves the last known error message given by the program
        Network_Error getLastErrorMsg();
};

#endif