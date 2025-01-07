#include "ncCodeTCP.h"

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
    server = INVALID_SOCKET;
    canListen = false;
    canReceive = false;
    canSend = false;
}

bool Server::initialize(const std::string& port_in)
{
    std::lock_guard general_lock(server_mtx);
    server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server == INVALID_SOCKET)
    {
        std::lock_guard err_lock(err_mtx);
        err = Socket_Creation_Failed;
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
    if (bind(server, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR)
    {
        std::lock_guard err_lock(err_mtx);
        err = Socket_Bind_Failed;
        closesocket(server);
        return false;
    }
    //Set socket to non-blocking
    u_long mode = 1;
    ioctlsocket(server, FIONBIO, &mode);
    return true;
}

void Server::start()
{
    //Server is already non-blocking at this point
    canListen = true;
    canReceive = true;
    canSend = true;
    size_t client_vector_size;
    u_long mode = 1;
    while (1)
    {
        { //Close listening if server is full
            std::unique_lock lock(client_vector_mtx);
            client_vector_size = client_vector.size();
            if (client_vector_size = max_connections) {canListen = false;}
            else {canListen = true;}
        }
        //May need to sleep depending on performance
        if (canListen)
        {
            {
                std::unique_lock lock(client_vector_mtx);
                client_vector_size = client_vector.size();
            }
            SOCKET newClient;
            {
                std::unique_lock socket_lock(server_mtx);
                newClient = accept(server, NULL, NULL);
                //invalid socket for client
            }
            //Accepts client if possible
            ioctlsocket(newClient, FIONBIO, &mode);
            {
                std::unique_lock lock(client_vector_mtx);
                client_vector.push_back(newClient);
            }
        } else if (canReceive) 
        {
            
        } else if (canSend) 
        {

        }
    }
}
