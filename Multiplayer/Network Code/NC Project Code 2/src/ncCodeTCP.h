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

enum Network_Errors
{
    None,
    Socket_Creation_Failed,
    Socket_Bind_Failed
};

namespace {
    WSAData NETWORK_WSADATA;
    #define NETWORK_TCP_DATA_OBJ_MAX_ARR_SIZE 1000
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

//Intialize WSA
bool Network_InitializeWSA();

//Server --
class Server
{
    public:
        //Function to handle incoming data. Set before calling initialize.
        std::function<void(const Network_TCP_Data_Obj& data)> data_Handler_Func;
        //Creates, binds the socket, giving loopback if needed
        bool initialize(const std::string& port_in);
        void start();
        void close();
        Server(u_short max_connections);
    private:
        int max_connections;
        std::mutex server_mtx;
        SOCKET server;
        Network_Errors err;
        std::mutex err_mtx;
        //Retrieves the last known error message given by the program
        Network_Errors getLastErrorMsg();
        std::vector<SOCKET> client_vector;
        std::mutex client_vector_mtx;
        //Start function modifiers
        std::atomic<bool> canListen;
        std::atomic<bool> canReceive;
        std::atomic<bool> canSend;
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
        Network_Errors err;
        std::mutex err_mtx;
        //Retrieves the last known error message given by the program
        Network_Errors getLastErrorMsg();
};


#endif