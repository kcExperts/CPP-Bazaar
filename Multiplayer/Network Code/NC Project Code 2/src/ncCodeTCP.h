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
    Server_Closed,
    Code_Could_Not_Be_Sent
};

//Holds the network error type and a mutex
struct Network_Error
{
    std::mutex mtx;
    Network_Error_Types type;
};

//Contains the Socket With a Lock
struct Socket_WL
{
    std::mutex mtx;
    SOCKET socket;
};

namespace {
    WSAData NETWORK_WSADATA;
    #define NETWORK_TCP_DATA_OBJ_MAX_ARR_SIZE 1000
    #define CLIENT_CONNECT_WAIT_TIME_S 5
    #define NETWORK_MAX_IDENTIFIER_STRING_SIZE 100
    #define NETWORK_MAX_CODE_SIZE 20
    #define NETWORK_CODE_SEPARATOR ':'
    #define NETWORK_CODE_GOOD std::string("G") + NETWORK_CODE_SEPARATOR
}

//Object that is sent through through the sockets
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

//Contains the vector with all the clients, a lock and the size of the vector
struct Server_Client_Vector
{
    std::vector<std::pair<std::string, SOCKET>> socket;
    std::mutex mtx;
    size_t size{0};
};

enum Network_Function_Mode
{
    SERVER_INDIVIDUAL,
    SERVER_INDEXED,
};

//Intializes WSA, should only be called once. Returns true if success and false if failure.
bool Network_InitializeWSA();

class Server
{
    public:
        /// Handles data received from a client. 
        /// 
        /// \param i The client identifier index. Use in get_identifier() to retrieve the client's identifier.
        /// \param data An object that contains data to be handled
        /// 
        /// This function can be set to a custom handler using the `data_Handler_Func` function pointer. If no custom function is provided, the default handler will print the client identifier and the data to the screen.
        std::function<void(const int& i, Network_TCP_Data_Obj& data)> data_Handler_Func;
        /// Handles when a client is diconnected from the server. 
        /// 
        /// \param i The client identifier index. Use in get_identifier() to retrieve the client's identifier.
        /// 
        /// This function can be set to a custom handler using the `client_disconnected` function pointer. If no custom function is provided, the default handler will log the event under `Client_Disconnected` code, which can be retrived by `get_last_error_msg`.
        std::function<void(const int& i)> client_disconnected;
        /// Handles when a client encounters a connection error. 
        /// 
        /// \param i The client identifier index. Use in get_identifier() to retrieve the client's identifier.
        /// 
        /// This function can be set to a custom handler using the `client_disconnected` function pointer. If no custom function is provided, the default handler will log the event under `Standard_Receive_Error_Occured` code, which can be retrived by `get_last_error_msg`.
        std::function<void(const int& i)> client_connection_error;
        /// Handles when the server encounters a broadcast error. 
        /// 
        /// \param i The client identifier index. Use in get_identifier() to retrieve the client's identifier.
        /// 
        /// This function can be set to a custom handler using the `client_disconnected` function pointer. If no custom function is provided, the default handler will log the event under `Standard_Broadcast_Error_Occured` code, which can be retrived by `get_last_error_msg`.
        std::function<void(const int& i)> broadcast_to_client_error;
        //Creates, binds the socket, giving loopback if needed
        bool initialize(const std::string& port_in, const std::string& code);
        void close();
        Server(u_short max_connections, bool SREO_flag);
        //Retrieves the last known error message given by the program
        Network_Error_Types get_last_error_msg();
        void dataReadyToSend();
        //Retrieves the current number of clients in the server
        size_t getCurrentServerSize();
        std::string get_identifier(const int& i);
        Network_TCP_Data_Obj data;
    private:
        std::thread listen_and_receive_thread;
        std::thread broadcast_thread;
        void listen_and_receive(Socket_WL& server, Server_Client_Vector& client_vector);
        void send_to_clients(Server_Client_Vector& client_vector, Network_TCP_Data_Obj& data_to_send);
        size_t max_connections;
        Socket_WL server;
        Network_Error err;
        Server_Client_Vector client_vector;
        //Start function modifiers
        std::atomic<bool> canListen;
        std::atomic<bool> toClose;
        std::string code;
        bool SREO_flag;
        std::mutex data_mtx;
        std::condition_variable data_cv;
};

enum Network_Client_Receive_Mode
{
    INITIALIZE,
    GENERAL
};  


class Client
{
    public:
        /// Handles data received from the server. 
        /// 
        /// \param data An object that contains data to be handled
        /// 
        /// This function can be set to a custom handler using the `data_Handler_Func` function pointer. If no custom function is provided, the default handler will print the broadcasted message to the screen.
        std::function<void(Network_TCP_Data_Obj& data)> data_Handler_Func;
        bool initialize(const std::string& ip_in, const std::string& port_in, const std::string& identifier, const std::string& code);
        void close();
        Client();
        //Retrieves the last known error message given by the program
        Network_Error_Types get_last_error_msg();

        Network_TCP_Data_Obj data;
        std::mutex data_mtx;
        std::condition_variable data_cv;
        
        void dataReadyToSend();
    private:
        Socket_WL client;
        Network_Error err;
        void receive(Network_Client_Receive_Mode mode, Socket_WL& client);
        void send_to_server(Socket_WL& client, Network_TCP_Data_Obj& data_to_send);
        std::thread receive_thread;
        std::thread send_thread;
        std::atomic<bool> isConnected;
        bool send_code_data(const std::string& identifier, const std::string& code);
        std::string code;
        std::string identifier;
        std::mutex wsa_mtx;
};


/// Receives data from a socket (server exclusive).
/// 
/// \param mode Specifies whether it scans through the client_vector (SERVER_INDEXED) or uses the provided socket (SERVER_INDIVIDUAL).
/// \param sock Used only if SERVER_INDIVIDUAL mode is selected.
/// \param index Used only if SERVER_INDEXED mode is selected.
/// \param client_vector The vector that contains the clients, should always be supplied even if not used.
/// \param data_recv An object that returns the received data.
/// \return The number of bytes received from the recv function.
int receive_from_socket(Network_Function_Mode mode, SOCKET* sock , size_t index, Server_Client_Vector* client_vector, Network_TCP_Data_Obj& data_recv);
/// Disconnects a socket from the server.
/// 
/// \param mode Specifies whether it scans through the client_vector (SERVER_INDEXED) or uses the provided socket (SERVER_INDIVIDUAL).
/// \param sock Used only if SERVER_INDIVIDUAL mode is selected.
/// \param index Used only if SERVER_INDEXED mode is selected.
/// \param client_vector The vector that contains the clients, should always be supplied even if not used.
void disconnect_socket(Network_Function_Mode mode, SOCKET* sock, size_t index, Server_Client_Vector* client_vector);
/// Sends data to a socket (server exclusive).
/// 
/// \param mode Specifies whether it scans through the client_vector (SERVER_INDEXED) or uses the provided socket (SERVER_INDIVIDUAL).
/// \param sock Used only if SERVER_INDIVIDUAL mode is selected.
/// \param index Used only if SERVER_INDEXED mode is selected.
/// \param client_vector The vector that contains the clients, should always be supplied even if not used.
/// \param data_to_send An object that contains the data to send.
/// \return The number of bytes sent from the send function.
int send_to_socket(Network_Function_Mode mode, SOCKET* sock, size_t index, Server_Client_Vector* client_vector, Network_TCP_Data_Obj& data_to_send);

#endif