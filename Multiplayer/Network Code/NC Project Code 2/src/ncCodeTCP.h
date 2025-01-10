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
    No_New_Error,
    Socket_Creation_Failed,
    Socket_Bind_Failed,
    Initialize_Listen_Failed,
    Receive_Select_Timeout,
    Client_Disconnected,
    Standard_Receive_Error_Occured,
    Standard_Broadcast_Error_Occured,
    Connection_Did_Not_Block,
    Connection_Timeout,
    Connection_Failed,
    Disconnected_From_Server,
    Receive_Error_Occured,
    Message_Send_Failed,
    Server_Closed
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
    #define CLIENT_CONNECT_WAIT_TIME_S 5
    #define MAX_USERNAME_STRING_SIZE 10
}

//extern std::mutex Network_TCP_Data_Obj_mtx;

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
        std::function<void(const int& i, Network_TCP_Data_Obj& data)> data_Handler_Func;
        std::function<void(const int& i)> client_disconnected;
        std::function<void(const int& i)> client_connection_error;
        std::function<void(const int& i)> broadcast_to_client_error;
        //Creates, binds the socket, giving loopback if needed
        bool initialize(const std::string& port_in);
        void close();
        Server(u_short max_connections);
        //Retrieves the last known error message given by the program
        Network_Error_Types getLastErrorMsg();
        void dataReadyToSend();
        //Retrieves the current number of clients in the server
        size_t getCurrentServerSize();

        Network_Data_Send_Obj data;
    private:
        std::thread listen_and_receive_thread;
        std::thread broadcast_thread;
        void listen_and_receive();
        void send_to_clients();
        size_t max_connections;
        Socket_WL server;
        Network_Error err;
        std::vector<std::pair<std::string, SOCKET>> client_vector;
        std::mutex client_vector_mtx;
        //Start function modifiers
        std::atomic<bool> canListen;
        std::atomic<bool> toClose;
        std::condition_variable dataReadyToSend_cv;

        //Broadcasts a message to all clients except for an exception
        void Broadcast(int exception, Network_Data_Send_Obj& info);
        void Disconnect(int& i);
        //Returns the number of bytes received from client i in the client_vector
        int ReceiveFrom(int i, Network_Data_Send_Obj& data_received);
};

class Client
{
    public:
        std::mutex connect_mtx;
        //Function to handle incoming data. Set before calling initialize.
        std::function<void(Network_TCP_Data_Obj& data)> data_Handler_Func;
        bool initialize(const std::string& ip_in, const std::string& port_in, const std::string& username);
        void close();
        Client();
        //Retrieves the last known error message given by the program
        Network_Error_Types getLastErrorMsg();
        Network_Data_Send_Obj data;
        void dataReadyToSend();
    private:
        Socket_WL client;
        Network_Error err;
        void receive(bool initialize);
        void send_to_server();
        std::thread receive_thread;
        std::thread send_thread;
        std::atomic<bool> isOperational;
        std::atomic<bool> isConnected;
        std::condition_variable dataReadyToSend_cv;
};

void testReceive(Socket_WL& client);


#endif