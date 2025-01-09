#include "ncCodeTCP.h"

std::mutex Network_TCP_Data_Obj_mtx;

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

namespace { //Utility function
    void Modify_Error(Network_Error_Types type, Network_Error& err)
    {
        std::unique_lock lock(err.mtx);
        err.type = type;
    }
}

Network_TCP_Data_Obj::Network_TCP_Data_Obj()
{
    timesToSendData = 0;
    array[0] = '\0';
    array_size = 0;
}

char Network_TCP_Data_Obj::getChar(int i) const
{
    std::lock_guard lock(Network_TCP_Data_Obj_mtx);
    if (i >= array_size) return '\0';
    return array[i];
}

std::string Network_TCP_Data_Obj::getString() const 
{
    std::lock_guard lock(Network_TCP_Data_Obj_mtx);
    return array;
}

bool Network_TCP_Data_Obj::setMessage(const std::string& str)
{
    if (str.length() > NETWORK_TCP_DATA_OBJ_MAX_ARR_SIZE - 1) return false;
    array_size = str.length();
    std::copy(str.begin(), str.end(), array);
    array[array_size] = '\0';
    return true;
}

bool Network_InitializeWSA()
{
    int wsaerr;
    //Desire version 2.2 of winsock2 (most recent as of this code)
    WORD wVersionRequested = MAKEWORD(2,2); 
    wsaerr = WSAStartup(wVersionRequested, &NETWORK_WSADATA);
    if (wsaerr != 0) return false;
    return true;
}

Server::Server(u_short max_connections)
{
    this->max_connections = max_connections;
    server.socket = INVALID_SOCKET;
    canListen = false;
    toClose = false;
    //Default functions
    data_Handler_Func = [](const int& i, Network_TCP_Data_Obj& data) {
        std::cout << "Client " << i << " sent: " << data.getString() << std::endl;
    };
    client_disconnected = [this](const int& i) {
        Modify_Error(Client_Disconnected, this->err);
    };
    client_connection_error = [this](const int& i) {
        Modify_Error(Standard_Receive_Error_Occured, this->err);
    };
    broadcast_to_client_error = [this](const int& i) {
        Modify_Error(Standard_Broadcast_Error_Occured, this->err);
    };
    Modify_Error(None, err);
}

size_t Server::getCurrentServerSize()
{
    std::unique_lock lock(client_vector_mtx);
    return client_vector.size();
}

bool Server::initialize(const std::string& port_in)
{
    std::lock_guard general_lock(server.mtx);
    server.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server.socket == INVALID_SOCKET)
    {
        Modify_Error(Socket_Creation_Failed, err);
        return false;
    }
    std::string ip = "0.0.0.0";
    sockaddr_in service;
    service.sin_family = AF_INET;
    if (std::stoi(port_in) == 0) //Optional loopback bind
    {
        ip = "127.0.0.1";
        std::string temp = "55555";
        InetPtonA(AF_INET, ip.c_str(), &service.sin_addr.S_un);
        service.sin_port = htons(std::stoi(temp));
    } else {
        InetPtonA(AF_INET, ip.c_str(), &service.sin_addr.S_un);
        service.sin_port = htons(std::stoi(port_in));
    }
    if (bind(server.socket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR)
    {
        Modify_Error(Socket_Bind_Failed, err);
        closesocket(server.socket);
        return false;
    }
    //Set socket to listening mode
    if (listen(server.socket, max_connections) == SOCKET_ERROR)
    {
        Modify_Error(Initialize_Listen_Failed, err);
        closesocket(server.socket);
        return false;
    }
    //Set socket to non-blocking
    u_long mode = 1;
    ioctlsocket(server.socket, FIONBIO, &mode);
    listen_and_receive_thread = std::thread(listen_and_receive, this);
    broadcast_thread = std::thread(send_to_clients, this);
    return true;
}

void Server::listen_and_receive()
{
    canListen = true;
    size_t client_vector_size;
    u_long mode = 1;
    Network_Data_Send_Obj data_Received;
    fd_set data_check;
    struct timeval select_wait;
    select_wait.tv_sec = 0;
    select_wait.tv_usec = 10000;
    while (1)
    {
        if (toClose) break;
        //Prevent clients from connecting if the server is full
        client_vector_size = getCurrentServerSize();
        if (client_vector_size == max_connections) {canListen = false;}
        else {canListen = true;}
        if (canListen) //Listen for connection
        {
            SOCKET newClient;
            std::unique_lock socket_lock(server.mtx);
            newClient = accept(server.socket, NULL, NULL);
            if (newClient != INVALID_SOCKET)
            {
                ioctlsocket(newClient, FIONBIO, &mode);
                std::unique_lock lock(client_vector_mtx);
                client_vector.push_back(newClient);
            }
        }
        //Receive a message
        SOCKET clientSock;
        for (int i = 0; i < client_vector_size; i++)
        {
            {
                std::unique_lock lock(client_vector_mtx);
                clientSock = client_vector[i];
                FD_ZERO(&data_check);
                FD_SET(clientSock, &data_check);
                int check = select(0, &data_check, NULL, NULL, &select_wait);
                // if (check == 0) {Modify_Error(Receive_Select_Timeout, err);} This check halts everything, explore later
                // else {put these around everything below}
                int bytes_recv = recv(clientSock, (char*)&data_Received.info, sizeof(data_Received.info), 0);
                if (bytes_recv == 0) //Client disconnected
                {
                    client_disconnected(i);
                    shutdown(clientSock, SD_BOTH);
                    closesocket(clientSock);
                    client_vector.erase(client_vector.begin() + i);
                    if (client_vector.empty()) break;
                    i--;
                    client_vector_size--; //Prevents WSAENOTSOCK (10038) error
                    continue;
                } else if (bytes_recv == SOCKET_ERROR)
                {
                    int ws_err = WSAGetLastError();
                    if (ws_err != WSAEWOULDBLOCK && ws_err != 0) client_connection_error(i);
                    continue;
                }
            }
            data_Handler_Func(i, data_Received.info);
            Broadcast(i, data_Received);
        }
    }
}

void Server::send_to_clients()
{
    while(1)
    {
        {
            std::unique_lock lock(data.mtx);
            dataReadyToSend_cv.wait(lock);
        }
        if (toClose) break;
        Broadcast(NETWORK_NOBODY, data);
    }
}

void Server::dataReadyToSend()
{
    std::unique_lock lock(data.mtx);
    dataReadyToSend_cv.notify_all();    
}

void Server::Broadcast(int exception, Network_Data_Send_Obj& info)
{
    std::unique_lock gen_lock(info.mtx);
    size_t server_size = getCurrentServerSize();
    for (int i = 0; i < server_size; i++)
    {
        if (i == exception) continue;
        std::unique_lock vec_lock(client_vector_mtx);
        SOCKET clientSock = client_vector[i];
        int byte_Count = send(clientSock, (char*)&info, sizeof(info), 0);
        if (byte_Count == SOCKET_ERROR || byte_Count == 0) {broadcast_to_client_error(i);}
    }
}

void Server::close()
{
    toClose = true;
    listen_and_receive_thread.join();
    dataReadyToSend();
    broadcast_thread.join();
}

Network_Error_Types Server::getLastErrorMsg()
{
    if (err.type == None) return No_New_Error;
    std::unique_lock lock(err.mtx);
    Network_Error_Types temp = err.type;
    err.type = None;
    return temp;
}

Client::Client()
{
    isOperational = false;
    isConnected = false;
    //Default function
    data_Handler_Func = [](Network_TCP_Data_Obj& data) {
        std::cout << "Server Broadcasted: " << data.getString() << std::endl;
    };
    Modify_Error(None, err);
}

bool Client::initialize(const std::string& ip_in, const std::string& port_in)
{
    //Create the socket
    client.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client.socket == INVALID_SOCKET) 
    {
        Modify_Error(Socket_Creation_Failed, err);
        return false;
    }
    u_long mode = 1;
    ioctlsocket(client.socket, FIONBIO, &mode);
    //Connect to the server
    sockaddr_in clientService;
    clientService.sin_family = AF_INET;
    //Loopback Connection Option
    std::string port = port_in;
    std::string ip = ip_in;
    if (std::stoi(port_in) == 0)
    {
        port = "55555";
        ip = "127.0.0.1";
    }
    InetPtonA(AF_INET, ip.c_str(), &clientService.sin_addr.S_un);
    clientService.sin_port = htons(std::stoi(port));
    fd_set client_check;
    FD_ZERO(&client_check);
    FD_SET(client.socket, &client_check);
    struct timeval select_wait;
    select_wait.tv_sec = CLIENT_CONNECT_WAIT_TIME_S;
    select_wait.tv_usec = 0;
    int connectRes = connect(client.socket, (SOCKADDR*)&clientService, sizeof(clientService));
    if (connectRes == 0) return true;
    connectRes = WSAGetLastError();
    if (connectRes != WSAEWOULDBLOCK)
    {
        Modify_Error(Connection_Did_Not_Block, err);
        return false;
    }
    connectRes = select(client.socket + 1, NULL, &client_check, NULL, &select_wait);
    if (connectRes <= 0)
    {
        Modify_Error(Connection_Timeout, err);
        return false;
    }
    socklen_t len = sizeof(connectRes);
    getsockopt(client.socket, SOL_SOCKET, SO_ERROR, (char*)&connectRes, &len);
    if (connectRes != 0)
    {
        Modify_Error(Connection_Failed, err);
        return false;
    }
    isConnected = true;
    isOperational = true;
    receive_thread = std::thread(receive, this);
    send_thread = std::thread(send_to_server, this);
    std::cout << "Success" << std::endl;
    return true;
}

void Client::receive()
{
    //Receive from server
    Network_Data_Send_Obj data_Received;
    fd_set data_check;
    struct timeval select_wait;
    select_wait.tv_sec = 0;
    select_wait.tv_usec = 10000;
    while(isOperational)
    {
        //Receiving data
        FD_ZERO(&data_check);
        FD_SET(client.socket, &data_check);
        int check = select(0, &data_check, NULL, NULL, &select_wait);
        if (check != 0) //Select has seen something
        {
            int bytesReceived = recv(client.socket, (char*)&data_Received, sizeof(data_Received), 0);
            if (bytesReceived == 0)
            {
                Modify_Error(Disconnected_From_Server, err);
                isConnected = false;
            } else if (bytesReceived == SOCKET_ERROR) {
                int err = WSAGetLastError();
                if (err != WSAEWOULDBLOCK && err != 0 && err != WSAECONNRESET)
                {
                    Modify_Error(Receive_Error_Occured, this->err);
                } else {
                    Modify_Error(Disconnected_From_Server, this->err);
                    isConnected = false;
                }
            }
            data_Handler_Func(data_Received.info);
        }
    }
}

void Client::send_to_server()
{
    while(1)
    {
        { //Send data
            std::unique_lock lock(data.mtx);
            dataReadyToSend_cv.wait(lock);
        }
        if (!isOperational) break;
        int byteCount = send(client.socket, (char*)&data.info, sizeof(data.info), 0);
        if (byteCount == SOCKET_ERROR)
        {
            if (WSAGetLastError())
            {
                Modify_Error(Disconnected_From_Server, err);
            } else {
                Modify_Error(Message_Send_Failed, err);
            }
        } else if (byteCount == 0) {
            Modify_Error(Server_Closed, err);
            isConnected = false;
        } 
    }
}

void Client::dataReadyToSend()
{
    std::unique_lock lock(data.mtx);
    dataReadyToSend_cv.notify_all();
}

void Client::close()
{
    isOperational = false;
    dataReadyToSend();
    receive_thread.join();
    send_thread.join();
    shutdown(client.socket, SD_BOTH);
    closesocket(client.socket);
}

Network_Error_Types Client::getLastErrorMsg()
{
    if (err.type == None) return No_New_Error;
    std::unique_lock lock(err.mtx);
    Network_Error_Types temp = err.type;
    err.type = None;
    return temp;
}