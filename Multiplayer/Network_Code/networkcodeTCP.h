#ifndef NETWORKCODETCP_H
#define NETWORKCODETCP_H

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
#include <functional> //Polymorphism for handling data
#include <utility> //Used for pairs

#include "ChatObject.h"

//Required to initialize winsock2 and the sockets
namespace { //Ensures these cannot be seen outside of the file
WSAData NETWORK_WSADATA;

//Sockets
}

//User does not need to concern themselves with this
struct Network_Server_Locks
{
    std::mutex client_vector_mtx;
    std::mutex client_vector_capacity_mtx;
    std::condition_variable client_vector_cv;
    std::mutex handling_data_mtx;
    std::condition_variable handling_data_cv;
    bool isServerFull;
    bool isServerEmpty;
    std::mutex sending_data_mtx;
    std::condition_variable sending_data_cv;
};

struct Network_Server_Client_Vector
{
    SOCKET socket;
    bool didSendData;
    std::string assignedUsername;
    bool isUsernamePresent;
};

struct Network_Server_Information
{
    std::vector<Network_Server_Client_Vector> client_vector;
    std::atomic<bool> isListening;
    std::atomic<bool> isReceiving;
    std::thread serverListeningThread; //1 second wait
    std::thread serverReceivingThread; //0.01 second wait
    std::thread serverBroadcastThread;
    std::string port;
    SOCKET server;
    int MAX_CONNECTIONS;
    std::function<void(const ChatObject& data)> data_Handler_Func; //Custom function to handle data
    std::atomic<bool> isBroadcasting;
    ChatObject dataToSend; //Edit, then notify to broadcast
};

//Ensure to lock before accessing buffer and to delete the read message
struct Network_Error_Msg_Buffer
{
    std::string msg_buffer;
    std::mutex msg_buffer_mtx;
};

struct Network_Client_Information
{
    std::string port;
    std::string ip;
    std::atomic<bool> isSending;
    std::atomic<bool> isReceiving;
    std::atomic<bool> isConnected;
    ChatObject dataToSend;
    std::thread clientSendThread;
    std::thread clientReceiveThread;
    SOCKET client;
    int CLIENT_CONNECT_TIMEOUT_SEC;
    std::function<void(const ChatObject& data)> data_Handler_Func; //Custom function to handle data
};

//User does not need to concern themselves with this
struct Network_Client_Locks
{
    std::mutex client_mtx;
    std::mutex winsock_mtx;
    std::mutex receive_mtx;
    std::condition_variable receive_cv;
    std::condition_variable send_data_cv;
};

//Should only be called once in any program. If it is already initialized, do not call.
void Network_InitializeWSA(Network_Error_Msg_Buffer& errors);
//Initializes the given fields with basic data, needs to be called before doing anything with the functions.
void Network_Server_Init(int max_allowed_connections, Network_Server_Locks& networkServerLocks, Network_Server_Information& networkServerInfo);
//Sets the port that the server will use
void Network_Server_Set_Port(const std::string& port_in, Network_Server_Information& networkServerInfo);
//Creates a socket, returning false if it fails. Use Network_GetLastError() for more details.
bool Network_Server_CreateSocket(Network_Server_Information& networkServerInfo);
//Binds the server to the given port and the 0.0.0.0 ip. Returns false if failed.
bool Network_Server_Bind(Network_Error_Msg_Buffer& errors, Network_Server_Information& networkServerInfo);
//Binds the server to the port 55555 and the loopback ip. Returns false if failed.
bool Network_Server_LoopbackBind(Network_Error_Msg_Buffer& errors, Network_Server_Information& networkServerInfo);
//Initializes threads used by the server, call after binding the server. Returns false if failed.
bool Network_Server_InitThreads(Network_Error_Msg_Buffer& errors, Network_Server_Locks& networkServerLocks, Network_Server_Information& networkServerInfo);
//Thread function. Receives data.
void Network_Server_Receive(Network_Error_Msg_Buffer& errors, Network_Server_Locks& networkServerLocks, Network_Server_Information& networkServerInfo); 
//Thread function. Listens for connections and accepts them.
void Network_Server_Listen(Network_Error_Msg_Buffer& errors, Network_Server_Locks& networkServerLocks, Network_Server_Information& networkServerInfo); 
//Thread function. Broadcasts data.
void Network_Server_Broadcast(Network_Error_Msg_Buffer& errors, Network_Server_Locks& networkServerLocks, Network_Server_Information& networkServerInfo); 
//Thread function. Mirrors the message received and sends it to all other connected clients.
void Network_Server_MirrorMessage(const ChatObject& data, Network_Error_Msg_Buffer& errors, Network_Server_Locks& networkServerLocks, Network_Server_Information& networkServerInfo);
//Notifies threads to perform an operation. Call this only once per loop iteration in the main program.
void Network_Server_Updates(Network_Server_Locks& networkServerLocks, Network_Server_Information& networkServerInfo);
//Notifies the server that a message is ready to be sent.
void Network_Server_SendMsg(Network_Server_Locks& networkServerLocks);
//Shuts down the server, including all threads.
void Network_Server_Shutdown(Network_Server_Locks& networkServerLocks, Network_Server_Information& networkServerInfo);
//Initializes the given fields with basic data, needs to be called before doing anything with the functions.
void Network_Client_Init(int client_connect_seconds_timeout, Network_Client_Information& networkClientInfo);
//Sets the ip that the client will use
void Network_Client_Set_Ip(const std::string& ip_in, Network_Client_Information& networkClientInfo);
//Sets the port that the client will use
void Network_Client_Set_Port(const std::string& port_in, Network_Client_Information& networkClientInfo);
//Creates a socket, returning false if it fails. Use Network_GetLastError() for more details.
bool Network_Client_CreateSocket(Network_Client_Locks& networkClientLocks, Network_Client_Information& networkClientInfo);
//Connects to the server using a given ip and port, returning false if fails.
bool Network_Client_ConnectToServer(Network_Error_Msg_Buffer& errors, Network_Client_Locks& networkClientLocks, Network_Client_Information& networkClientInfo);
//Connects to the server using the port 55555 and the loopback ip, server needs to be initialized using loopbackbind
bool Network_Client_LoopbackConnect(Network_Error_Msg_Buffer& errors, Network_Client_Locks& networkClientLocks, Network_Client_Information& networkClientInfo);
//Initializes threads used by the client, call after connecting to the server successfully.
void Network_Client_InitThreads(Network_Error_Msg_Buffer& errors, Network_Client_Locks& networkClientLocks, Network_Client_Information& networkClientInfo);
//Thread function. Sends data to server.
void Network_Client_SendToServer(Network_Error_Msg_Buffer& errors, Network_Client_Locks& networkClientLocks, Network_Client_Information& networkClientInfo); //Thread
//Thread function. Receives data from the server.
void Network_Client_ReceiveFromServer(Network_Error_Msg_Buffer& errors, Network_Client_Locks& networkClientLocks, Network_Client_Information& networkClientInfo); //Thread
//Notifies threads to perform an operation. Call this only once per loop iteration in the main program.
void Network_Client_Updates(Network_Client_Locks& networkClientLocks);
//Shuts down the client, including all threads.
void Network_Client_Shutdown(Network_Client_Locks& networkClientLocks, Network_Client_Information& networkClientInfo);
//Default function to handle data.
void Network_DefaultHandleData(const ChatObject& data); //Default data handler, assigned in init
//Returns the last generated error.
const std::string Network_GetLastError(Network_Error_Msg_Buffer& errors);

#endif