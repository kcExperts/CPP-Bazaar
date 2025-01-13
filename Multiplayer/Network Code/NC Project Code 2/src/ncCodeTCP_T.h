#ifndef NCCODETCP_T_H
#define NCCODETCP_T_H

#include <mutex>
#include <condition_variable>
#include <functional>
#include <string>
#include <atomic>
#include <algorithm>
#include <vector>
#include <iostream>
#include <thread>

#define NETWORK_TCP_DATA_OBJ_MAX_ARR_SIZE 1000
#define CLIENT_CONNECT_WAIT_TIME_S 5
#define NETWORK_MAX_IDENTIFIER_STRING_SIZE 100
#define NETWORK_MAX_CODE_SIZE 20
#define NETWORK_CODE_SEPARATOR ':'
#define NETWORK_CODE_GOOD std::string("G") + NETWORK_CODE_SEPARATOR

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
    Server_Closed,
    Code_Could_Not_Be_Sent
};

//Holds the network error type and a mutex
struct Network_Error
{
    std::mutex mtx;
    Network_Error_Types type;
};

extern Network_Error error_codes;

Network_Error_Types get_last_error_msg(Network_Error& error);

enum Network_Function_Mode
{
    SERVER_INDIVIDUAL,
    SERVER_INDEXED,
    CLIENT_GEN
};

enum Network_Client_Receive_Mode
{
    INITIALIZE,
    GENERAL
};  

//Object that is sent through through the sockets
class Network_TCP_Data_Obj
{
    private:
        int type;
        char array[NETWORK_TCP_DATA_OBJ_MAX_ARR_SIZE];
        int array_size;
    public:
        Network_TCP_Data_Obj();
        std::string getString() const;
        bool setMessage(const std::string& str);
        int getType() const;
        void setType(int type);
};

#if defined(_WIN32)

    #define WIN32_LEAN_AND_MEAN
    #include <winsock2.h>
    #include <ws2tcpip.h>
    
    namespace {WSAData NETWORK_WSADATA;}

    //Contains the Socket With a Lock
    struct Socket_WL
    {
        std::mutex mtx;
        SOCKET socket;
    };

    //Contains the vector with all the clients, a lock and the size of the vector
    struct Server_Client_Vector
    {
        std::vector<std::pair<std::string, SOCKET>> socket;
        std::mutex mtx;
        size_t size{0};
    };

    //Intializes WSA, should only be called once. Returns true if success and false if failure.
    bool Network_InitializeWSA();

    class Server
    {
        public:
            std::function<void(const int& i, Network_TCP_Data_Obj& data)> data_Handler_Func;
            std::function<void(const int& i)> client_disconnected;
            std::function<void(const int& i)> client_connection_error;
            std::function<void(const int& i)> broadcast_to_client_error;
            Server(u_short max_connections, bool SREO_flag);
            bool initialize(const std::string& port_in, const std::string& code);
            void close();
            void data_ready_to_send();
            size_t get_current_server_size();
            std::string get_identifier(const int& i);
            bool set_data_to_send(const std::string& data_in, int data_type);
        private:
            Network_TCP_Data_Obj data;
            std::thread listen_and_receive_thread;
            std::thread broadcast_thread;
            size_t max_connections;
            Socket_WL server;
            Server_Client_Vector client_vector;
            std::atomic<bool> canListen;
            std::atomic<bool> toClose;
            std::string code;
            bool SREO_flag;
            std::mutex data_mtx;
            std::condition_variable data_cv;
            void listen_and_receive(Socket_WL& server, Server_Client_Vector& client_vector); //Thread function
            void send_to_clients(Server_Client_Vector& client_vector, Network_TCP_Data_Obj& data_to_send); //Thread function
    };

    class Client
    {
        public:
            std::function<void(Network_TCP_Data_Obj& data)> data_Handler_Func;
            Client();
            bool initialize(const std::string& ip_in, const std::string& port_in, const std::string& identifier, const std::string& code);
            void close();
            void data_ready_to_send();
            bool set_data_to_send(const std::string& data_in, int data_type);
        private:
            Network_TCP_Data_Obj data;
            std::mutex data_mtx;
            std::condition_variable data_cv;
            Socket_WL client;
            std::string code;
            std::string identifier;
            std::mutex wsa_mtx;
            std::thread receive_thread;
            std::thread send_thread;
            std::atomic<bool> isConnected;
            void receive(Network_Client_Receive_Mode mode, Socket_WL& client);
            void send_to_server(Socket_WL& client, Network_TCP_Data_Obj& data_to_send);
    };
    Network_Error_Types create_socket(Socket_WL& sock);
    Network_Error_Types bind_socket(Socket_WL& sock, const std::string & port_in, int max_connections);
    int receive_from_socket(Network_Function_Mode mode, SOCKET* sock , size_t index, Server_Client_Vector* client_vector, Network_TCP_Data_Obj& data_recv);
    void disconnect_socket(Network_Function_Mode mode, SOCKET* sock, size_t index, Server_Client_Vector* client_vector);
    int send_to_socket(Network_Function_Mode mode, SOCKET* sock, size_t index, Server_Client_Vector* client_vector, Network_TCP_Data_Obj& data_to_send);


#elif defined(__linux__)

    
#endif



#endif