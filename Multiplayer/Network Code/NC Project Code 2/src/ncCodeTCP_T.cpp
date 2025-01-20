#include "ncCodeTCP_T.h"

namespace { //Utility function
    void Modify_Error(Network_Error_Types type, Network_Error& err)
    {
        std::unique_lock<std::mutex> lock(err.mtx);
        err.type = type;
    }
}

Network_Error error_codes;

void Server::data_ready_to_send()
{
    std::unique_lock<std::mutex> lock(data_mtx);
    data_cv.notify_all();    
}

size_t Server::get_current_server_size()
{
    std::unique_lock<std::mutex> vec_lock(client_vector.mtx);
    return client_vector.size;
}

std::string Server::get_identifier(const int& i)
{
    std::unique_lock<std::mutex> vec_lock(client_vector.mtx);
    return client_vector.socket.at(i).first;
}

bool Server::set_data_to_send(const std::string& data_in, int data_type)
{
    data.setType(data_type);
    return data.setMessage(data_in);
}

void Client::data_ready_to_send()
{
    std::unique_lock<std::mutex> lock(data_mtx);
    data_cv.notify_all();
}

bool Client::set_data_to_send(const std::string& data_in, int data_type)
{
    data.setType(data_type);
    return data.setMessage(data_in);
}

#if defined(_WIN32)

    bool Network_InitializeWSA()
    {
        int wsaerr;
        //Desire version 2.2 of winsock2 (most recent as of this code)
        WORD wVersionRequested = MAKEWORD(2,2); 
        wsaerr = WSAStartup(wVersionRequested, &NETWORK_WSADATA);
        if (wsaerr != 0) return false;
        return true;
    }

    Server::Server(u_short max_connections, bool SREO_flag)
    {
        this->SREO_flag = SREO_flag;
        this->max_connections = max_connections;
        server.socket = INVALID_SOCKET;
        canListen = false;
        toClose = false;
        //Default functions
        data_Handler_Func = [this](const int& i, Network_TCP_Data_Obj& data) {
            std::cout << client_vector.socket.at(i).first << ": " << data.getString() << std::endl;
        };
        client_disconnected = [this](const int& i) {
            Modify_Error(Client_Disconnected, error_codes);
        };
        client_connection_error = [this](const int& i) {
            Modify_Error(Standard_Receive_Error_Occured, error_codes);
        };
        broadcast_to_client_error = [this](const int& i) {
            Modify_Error(Standard_Broadcast_Error_Occured, error_codes);
        };
        Modify_Error(None, error_codes);
    }


    bool Server::initialize(const std::string & port_in, const std::string& code)
    {
        Network_Error_Types information;
        this->code = code;
        information = create_socket(server);
        if (information != None)
        {
            Modify_Error(information, error_codes);
            return false;
        }
        //Bind the socket
        information = bind_socket(server, port_in, this->max_connections);
        if (information != None)
        {
            Modify_Error(information, error_codes);
            return false;
        }
        //Open the threads
        canListen = true;
        toClose = false;
        listen_and_receive_thread = std::thread(listen_and_receive, this, std::ref(server), std::ref(client_vector));
        broadcast_thread = std::thread(send_to_clients, this, std::ref(client_vector), std::ref(data));
        return true;
    }

    void Server::close()
    {
        toClose = true;
        listen_and_receive_thread.join();
        data_ready_to_send();
        broadcast_thread.join();
    }

    void Server::listen_and_receive(Socket_WL& server, Server_Client_Vector& client_vector)
    {
        size_t client_vector_size;
        Network_TCP_Data_Obj data_Received;
        while (1)
        {
            if (toClose) break;
            {
                std::unique_lock<std::mutex> vector_lock(client_vector.mtx);
                client_vector_size = client_vector.size;
            }
            if (client_vector_size == max_connections) {canListen = false;} //Halt listening if server  is full
            else {canListen = true;}
            //Listen
            if (canListen)
            {
                SOCKET potential_client;
                {
                    std::unique_lock<std::mutex> socket_lock(server.mtx);
                    potential_client = accept(server.socket, NULL, NULL);
                }
                if (potential_client != INVALID_SOCKET) 
                { //We have a new client
                    bool good_client = false;
                    std::string identifier;
                    Network_TCP_Data_Obj temporary_pipeline = Network_TCP_Data_Obj();
                    int bytes_recv = receive_from_socket(SERVER_INDIVIDUAL, &potential_client, 0, NULL, temporary_pipeline);
                    std::string message = temporary_pipeline.getString();
                    size_t separator_pos = message.find(NETWORK_CODE_SEPARATOR);
                    if (separator_pos != std::string::npos)
                    {
                        std::string code = message.substr(0, separator_pos);
                        identifier = message.substr(separator_pos + 1);
                        if (code != this->code)
                        {
                            temporary_pipeline.setMessage("Connection Request Rejected: Invalid Code");
                            send_to_socket(SERVER_INDIVIDUAL, &potential_client, 0, NULL, temporary_pipeline);
                            disconnect_socket(SERVER_INDIVIDUAL, &potential_client, 0, NULL);
                        } else if (identifier.find(NETWORK_CODE_SEPARATOR) != std::string::npos || identifier.length() > NETWORK_MAX_IDENTIFIER_STRING_SIZE || identifier.length() == 0)
                        {
                            temporary_pipeline.setMessage("Connection Request Rejected: Invalid Identifier");
                            send_to_socket(SERVER_INDIVIDUAL, &potential_client, 0, NULL, temporary_pipeline);
                            disconnect_socket(SERVER_INDIVIDUAL, &potential_client, 0, NULL);
                        } else {
                            temporary_pipeline.setMessage(NETWORK_CODE_GOOD);
                            int bytes = send_to_socket(SERVER_INDIVIDUAL, &potential_client, 0, NULL, temporary_pipeline); 
                            good_client = true;
                        }
                    }
                    if (good_client)
                    {
                        u_long mode = 1;
                        ioctlsocket(potential_client, FIONBIO, &mode); //Set new client to non-blocking   
                        {
                            std::unique_lock<std::mutex> vector_lock(client_vector.mtx);
                            client_vector.socket.push_back(std::pair(identifier, potential_client)); //Change to accomodate for username
                            client_vector.size++;
                        }
                        client_vector_size++;                
                    }              
                }
            }
            //Check for a message to be received
            for (size_t i = 0; i < client_vector_size; i++)
            {
                int bytes_recv = receive_from_socket(SERVER_INDEXED, NULL, i, &client_vector, data_Received);
                if (bytes_recv == 0) //Client disconnected
                {
                    client_disconnected(i);
                    disconnect_socket(SERVER_INDEXED, NULL, i, &client_vector);
                    i--;
                    client_vector_size--;
                } else if (bytes_recv == SOCKET_ERROR)
                {
                    if (WSAGetLastError() == WSAEWOULDBLOCK) continue;
                    client_connection_error(i);
                    if (SREO_flag)
                    {
                        disconnect_socket(SERVER_INDEXED, NULL, i, &client_vector);
                        i--;
                        client_vector_size--;
                    }
                } else { //Data received, broadcast to other clients
                    data_Handler_Func(i, data_Received);
                    for (size_t k = 0; k < client_vector_size; k++)
                    {
                        if (k != i)
                        {
                            int bytes_sent = send_to_socket(SERVER_INDEXED, NULL, k, &client_vector, data_Received);
                            if (bytes_sent == SOCKET_ERROR || bytes_sent == 0) {broadcast_to_client_error(k);}
                        }
                    }
                }
            }
        }
    }

    void Server::send_to_clients(Server_Client_Vector& client_vector, Network_TCP_Data_Obj& data_to_send)
    {
        int client_vec_size = 0;
        while(1)
        {
            {
                std::unique_lock<std::mutex> wait_lock(data_mtx);
                data_cv.wait(wait_lock);
            }
            if (toClose) break;
            {
                std::unique_lock<std::mutex> vec_lock(client_vector.mtx);
                client_vec_size = client_vector.size;
            }
            for (size_t i = 0; i < client_vec_size; i++)
            {
                int bytes_sent = send_to_socket(SERVER_INDEXED, NULL, i, &client_vector, data_to_send);
                if (bytes_sent == SOCKET_ERROR || bytes_sent == 0) {broadcast_to_client_error(i);}
            }
        }
    }

    Client::Client()
    {
        isConnected = false;
        //Default function
        data_Handler_Func = [](Network_TCP_Data_Obj& data) {
            std::cout << "Server Broadcasted: " << data.getString() << std::endl;
        };
        Modify_Error(None, error_codes);
    }

    bool Client::initialize(const std::string& ip_in, const std::string& port_in, const std::string& identifier, const std::string& code)
    {
        //Create the socket
        this->identifier = identifier;
        this->code = code;
        create_socket(client);
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
            Modify_Error(Connection_Did_Not_Block, error_codes);
            return false;
        }
        connectRes = select(client.socket + 1, NULL, &client_check, NULL, &select_wait);
        if (connectRes <= 0)
        {
            Modify_Error(Connection_Timeout, error_codes);
            return false;
        }
        socklen_t len = sizeof(connectRes);
        getsockopt(client.socket, SOL_SOCKET, SO_ERROR, (char*)&connectRes, &len);
        if (connectRes != 0)
        {
            Modify_Error(Connection_Failed, error_codes);
            return false;
        }
        //Send code
        isConnected = true;
        Network_TCP_Data_Obj code_and_identifier;
        std::string temp = code + NETWORK_CODE_SEPARATOR + identifier;
        code_and_identifier.setMessage(temp);
        int bytes_sent;
        receive_thread = std::thread(receive, this, INITIALIZE, std::ref(client));
        {
            std::unique_lock<std::mutex> socket_lock(client.mtx);
            bytes_sent = send(client.socket, (char*)&code_and_identifier, sizeof(code_and_identifier), 0);
            if (bytes_sent == SOCKET_ERROR)
            {
                Modify_Error(Code_Could_Not_Be_Sent, error_codes);
                receive_thread.join();
                return false;
            }
        }
        receive_thread.join();
        if (!isConnected) return false;
        //Start general threads
        receive_thread = std::thread(receive, this, GENERAL, std::ref(client));
        send_thread = std::thread(send_to_server, this, std::ref(client), std::ref(data));
        return true;
    }

    void Client::c_close()
    {
        isConnected = false;
        data_ready_to_send();
        receive_thread.join();
        send_thread.join();
        shutdown(client.socket, SD_BOTH);
        closesocket(client.socket);
    }

    void Client::receive(Network_Client_Receive_Mode mode, Socket_WL& client)
    {
        u_short time = 0;
        Network_TCP_Data_Obj data_Received = Network_TCP_Data_Obj(); //Investigate overloads
        fd_set data_check;
        FD_ZERO(&data_check);
        struct timeval select_wait;
        select_wait.tv_sec = 0;
        select_wait.tv_usec = 10000;
        while(isConnected)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            int check, bytes_Received, wsa_error;
            if (mode == INITIALIZE)
            {
                time++;
                if (time > 100) {isConnected = false; break;}
            }
            //Select check
            {
                std::unique_lock<std::mutex> socket_lock(client.mtx);
                FD_SET(client.socket, &data_check);
                check = select(0, &data_check, NULL, NULL, &select_wait);
                FD_ZERO(&data_check);
            }
            if (check > 0)
            {
                {
                    std::unique_lock<std::mutex> socket_lock(client.mtx);
                    bytes_Received = recv(client.socket, (char*)&data_Received, sizeof(data_Received), 0);
                }
                if (bytes_Received == 0)
                {
                    Modify_Error(Disconnected_From_Server, error_codes);
                    isConnected = false;
                    continue;
                } else if (bytes_Received == SOCKET_ERROR)
                {
                    {
                        std::unique_lock<std::mutex> wsa_lock(wsa_mtx);
                        wsa_error = WSAGetLastError();
                    }
                    if (wsa_error != WSAEWOULDBLOCK && wsa_error != 0 && wsa_error != WSAECONNRESET) {Modify_Error(Receive_Error_Occured,  error_codes);}
                    else {
                        Modify_Error(Disconnected_From_Server, error_codes);
                        isConnected = false;
                        continue;
                    }
                }
                if (mode == INITIALIZE)
                {
                    std::string connection_success = NETWORK_CODE_GOOD;
                    if (data_Received.getString() == connection_success) {break;}
                    else {
                        data_Handler_Func(data_Received);
                        isConnected = false; 
                        break;
                    }
                } else {
                    data_Handler_Func(data_Received);
                }
            }
        }
    }

    void Client::send_to_server(Socket_WL& client, Network_TCP_Data_Obj& data_to_send)
    {
        while(1)
        {
            int bytes_sent, wsa_error;
            {
                std::unique_lock<std::mutex> wait_lock(data_mtx);
                data_cv.wait(wait_lock);
            }
            if (!isConnected) break;
            {
                std::unique_lock<std::mutex> socket_lock(client.mtx);
                bytes_sent = send(client.socket, (char*)&data_to_send, sizeof(data_to_send), 0);
            }
            if (bytes_sent == SOCKET_ERROR)
            {
                {
                    std::unique_lock<std::mutex> wsa_lock(wsa_mtx);
                    wsa_error = WSAGetLastError();
                }
                if (wsa_error)
                {
                    Modify_Error(Disconnected_From_Server, error_codes);
                    isConnected = false;
                } else {
                    Modify_Error(Message_Send_Failed, error_codes);
                }
            } else if (bytes_sent == 0) {
                Modify_Error(Server_Closed, error_codes);
                isConnected = false;
            }
        }
    }

#elif defined(__linux__)






#endif

#if defined(_WIN32)

    Network_Error_Types create_socket(Socket_WL& sock)
    {
        std::unique_lock<std::mutex> socket_lock(sock.mtx);
        sock.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock.socket == INVALID_SOCKET) return Socket_Creation_Failed;
        return None;
    }

    Network_Error_Types bind_socket(Socket_WL& sock, const std::string & port_in, int max_connections)
    {
        std::string ip;
        sockaddr_in service; //Contains connection details for the sokcet
        service.sin_family = AF_INET;
        if (std::stoi(port_in) == 0) //Optional loopback bind
        {
            ip = "127.0.0.1";
            std::string loopback_port = "55555";
            InetPtonA(AF_INET, ip.c_str(), &service.sin_addr.S_un);
            service.sin_port = htons(std::stoi(loopback_port));
        } else { //Standard bind
            ip = "0.0.0.0"; //access all ipv4 addresses on local machine
            InetPtonA(AF_INET, ip.c_str(), &service.sin_addr.S_un);
            service.sin_port = htons(std::stoi(port_in));
        }
        std::unique_lock<std::mutex> socket_lock(sock.mtx);
        if (bind(sock.socket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) return Socket_Bind_Failed;
        if (listen(sock.socket, max_connections) == SOCKET_ERROR) //Put socket in listening state
        {
            shutdown(sock.socket, SD_BOTH);
            closesocket(sock.socket);
            return Initialize_Listen_Failed;
        }
        //Set socket to non-blocking
        u_long mode = 1;
        ioctlsocket(sock.socket, FIONBIO, &mode);
        return None;
    }

    int receive_from_socket(Network_Function_Mode mode, SOCKET* sock ,size_t index, Server_Client_Vector* client_vector, Network_TCP_Data_Obj& data_recv)
    {
        fd_set data_check;
        struct timeval select_wait;
        select_wait.tv_sec = 0;
        select_wait.tv_usec = 10000;
        FD_ZERO(&data_check);
        if (mode == SERVER_INDIVIDUAL)
        {
            FD_SET(*sock, &data_check);
            int select_output = select(0, &data_check, NULL, NULL, &select_wait);
            //do checks for select_output
            return recv(*sock, (char*)&data_recv, sizeof(data_recv), 0);
        } 
        std::unique_lock<std::mutex> vector_lock(client_vector->mtx);
        FD_SET(client_vector->socket.at(index).second, &data_check);
        int select_output = select(0, &data_check, NULL, NULL, &select_wait);
        //do checks for select_output
        return recv(client_vector->socket.at(index).second, (char*)&data_recv, sizeof(data_recv), 0);
    }

    void disconnect_socket(Network_Function_Mode mode, SOCKET* sock, size_t index, Server_Client_Vector* client_vector)
    {
        if (mode == SERVER_INDIVIDUAL)
        {
            shutdown(*sock, SD_BOTH);
            closesocket(*sock);
        } else {
            std::unique_lock<std::mutex> vec_lock(client_vector->mtx);
            shutdown(client_vector->socket.at(index).second, SD_BOTH);
            closesocket(client_vector->socket.at(index).second);
            client_vector->socket.erase(client_vector->socket.begin() + index);
            client_vector->size--;
        }
    }

    int send_to_socket(Network_Function_Mode mode, SOCKET* sock, size_t index, Server_Client_Vector* client_vector, Network_TCP_Data_Obj& data_to_send)
    {
        if (mode == SERVER_INDIVIDUAL) return send(*sock, (char*)&data_to_send, sizeof(data_to_send), 0);
        std::unique_lock<std::mutex> vec_lock(client_vector->mtx);
        return send(client_vector->socket.at(index).second, (char*)&data_to_send, sizeof(data_to_send), 0);
    }

#elif defined(__linux__)
    
#endif


Network_Error_Types get_last_error_msg(Network_Error& error)
{
    if (error.type == None) return No_New_Error;
    std::unique_lock<std::mutex> lock(error.mtx);
    Network_Error_Types temp = error.type;
    error.type = None;
    return temp;
}

std::mutex Network_TCP_Data_Obj_mtx;

Network_TCP_Data_Obj::Network_TCP_Data_Obj()
{
    type = 0;
    array[0] = '\0';
    array_size = 0;
}

std::string Network_TCP_Data_Obj::getString() const
{
    std::lock_guard<std::mutex> lock(Network_TCP_Data_Obj_mtx);
    return array;
}

int Network_TCP_Data_Obj::getType() const
{
    std::lock_guard<std::mutex> lock(Network_TCP_Data_Obj_mtx);
    return type;
}

void Network_TCP_Data_Obj::setType(int type)
{
    this->type = type;
}

bool Network_TCP_Data_Obj::setMessage(const std::string& str)
{
    if (str.length() > NETWORK_TCP_DATA_OBJ_MAX_ARR_SIZE - 1) return false;
    array_size = str.length();
    std::copy(str.begin(), str.end(), array);
    array[array_size] = '\0';
    return true;
}