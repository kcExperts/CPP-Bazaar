#include "ncCodeTCP.h"

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
    dataReadyToSend = false;
    toClose = false;
    //Default functions
    data_Handler_Func = [](const int& i, const Network_TCP_Data_Obj& data) {
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
    u_short port = std::stoi(port_in); //Assume port_in is valid
    if (port == 0) //Optional loopback bind
    {
        ip = "127.0.0.1";
        port = 55555;
    }
    sockaddr_in service;
    service.sin_family = AF_INET;
    InetPtonA(AF_INET, ip.c_str(), &service.sin_addr.S_un);
    service.sin_port = htons(port);
    if (bind(server.socket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR)
    {
        Modify_Error(Socket_Bind_Failed, err);
        closesocket(server.socket);
        return false;
    }
    //Set socket to non-blocking
    u_long mode = 1;
    ioctlsocket(server.socket, FIONBIO, &mode);
    logic = std::thread(start);
    return true;
}

void Server::start() //May need to sleep depending on performance
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
            if (newClient == INVALID_SOCKET) {Modify_Error(Listen_Error, err);}
            else
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
            std::unique_lock lock(client_vector_mtx);
            clientSock = client_vector[i];
            FD_ZERO(&data_check);
            FD_SET(clientSock, &data_check);
            int check = select(0, &data_check, NULL, NULL, &select_wait);
            if (check == 0) {Modify_Error(Receive_Select_Timeout, err);}
            else 
            {
                std::unique_lock lock(client_vector_mtx);
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
                } else if (bytes_recv == SOCKET_ERROR)
                {
                    int ws_err = WSAGetLastError();
                    if (ws_err != WSAEWOULDBLOCK && ws_err != 0) client_connection_error(i);
                    else {Modify_Error(Unknown_Receive_Error_Occured, err);}
                } else 
                {
                    data_Handler_Func(i, data_Received.info);
                    Broadcast(i, data_Received);
                }
            }
        }
        //Send a message
        if (dataReadyToSend)
        {
            Broadcast(NETWORK_NOBODY, server_data);
            dataReadyToSend = false;
        }
    }
}

void Server::Broadcast(int exception, const Network_Data_Send_Obj& info)
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
    logic.join();
}