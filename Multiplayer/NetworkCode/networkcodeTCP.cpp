#include "networkcodeTCP.h"

void Network_Server_Receive(Network_Error_Msg_Buffer& errors, Network_Server_Locks& networkServerLocks, Network_Server_Information& networkServerInfo) //This function executes a LOT of times per second, may put cap in future (depending on cpu obliteration)
{
    ChatObject dataReceived;
    networkServerInfo.isReceiving = true;
    size_t client_vector_size = 0;
    SOCKET clientSoc;
    //We wait 10ms to check if more data will arrive
    fd_set data_check;
    struct timeval select_wait;
    select_wait.tv_sec = 0;
    select_wait.tv_usec = 10000; //Creates waiting (10 ms)
    while (networkServerInfo.isReceiving)
    {
        {   //Get capacity
            std::unique_lock lock(networkServerLocks.client_vector_mtx);
            client_vector_size = networkServerInfo.client_vector.size();
        }
        if (client_vector_size == 0)
        { //Halt thread if the server is empty
            std::unique_lock lock(networkServerLocks.client_vector_capacity_mtx);
            networkServerLocks.isServerEmpty = true;
            networkServerLocks.client_vector_cv.wait(lock, [&networkServerLocks]{return !networkServerLocks.isServerEmpty;}); //Capture by reference
            if (!networkServerInfo.isReceiving) break; //End thread (for shutdown)
        } 
        for (int i = 0; i < client_vector_size; i++)
        {
            {   //Get the size and client
                std::unique_lock lock(networkServerLocks.client_vector_mtx);
                client_vector_size = networkServerInfo.client_vector.size();
                clientSoc = networkServerInfo.client_vector[i].socket;
            }
            FD_ZERO(&data_check); //Clear the previous socket from the set, as we only want to check the current one
            FD_SET(clientSoc, &data_check); //Add current socket to set
            int check = select(0, &data_check, NULL, NULL, &select_wait);
            //TODO: Verify that check = 0 to continue, if not, it should say that receiving message failed cause timeout and continue to next socket
            //if (check == 0) continue;
            int bytesReceived = recv(clientSoc, (char*)&dataReceived, sizeof(dataReceived), 0);
            if (bytesReceived == 0) //Client has disconnected
            {
                {
                    std::scoped_lock broadcast_lock(errors.msg_buffer_mtx, networkServerLocks.client_vector_mtx, networkServerLocks.client_vector_capacity_mtx);
                    errors.msg_buffer = (networkServerInfo.client_vector[i].assignedUsername + " has disconnected");
                }
                shutdown(clientSoc, SD_BOTH);
                closesocket(clientSoc);
                std::scoped_lock lock(networkServerLocks.client_vector_mtx, networkServerLocks.client_vector_capacity_mtx);
                networkServerInfo.client_vector.erase(networkServerInfo.client_vector.begin() + i); //Delete socket from client_vector
                if (networkServerInfo.client_vector.empty()) break;
                i--;
                client_vector_size--; //Prevent WSAENOTSOCK (10038) error
                continue;
            }
            if (bytesReceived == SOCKET_ERROR) //Receive failed for some reason
            {
                std::unique_lock<std::mutex> lock(errors.msg_buffer_mtx);
                int err = WSAGetLastError();
                if (err != WSAEWOULDBLOCK && err != 0) //Log the error if it is not WSAEWOULDBLOCK
                {
                    std::scoped_lock broadcast_lock(errors.msg_buffer_mtx, networkServerLocks.client_vector_mtx, networkServerLocks.client_vector_capacity_mtx);
                    errors.msg_buffer = (networkServerInfo.client_vector[i].assignedUsername + " has had a connection error occur");
                }
                continue;
            }
            { //Check if a username has been assigned to the client (client needs to give valid username)
                std::scoped_lock username_lock(networkServerLocks.client_vector_mtx, networkServerLocks.client_vector_capacity_mtx);
                if (!networkServerInfo.client_vector[i].isUsernamePresent)
                {
                    networkServerInfo.client_vector[i].assignedUsername = dataReceived.getUsername();
                    networkServerInfo.client_vector[i].isUsernamePresent = true;
                }
            }
            networkServerInfo.data_Handler_Func(dataReceived); //Process data
            { //Broadcast the data to all the other sockets (excluding the one that send the message)
                std::scoped_lock broadcast_lock(networkServerLocks.client_vector_mtx, networkServerLocks.client_vector_capacity_mtx);
                networkServerInfo.client_vector[i].didSendData = true; //Ensures that broadcast does not send to this socket
                Network_Server_MirrorMessage(dataReceived, errors, networkServerLocks, networkServerInfo);
                networkServerInfo.client_vector[i].didSendData = false;
            }
        }
    }
}

void Network_DefaultHandleData(const ChatObject& data)
{
    std::cout << data.getUsername() << ": " << data.getMessage() << std::endl;
}

void Network_Server_Listen(Network_Error_Msg_Buffer& errors, Network_Server_Locks& networkServerLocks, Network_Server_Information& networkServerInfo) //Error occurs here for CV
{
    networkServerInfo.isListening = true;
    //Set server and new client sockets to non-blocking
    u_long mode = 1; //Non-blocking mode 
    ioctlsocket(networkServerInfo.server, FIONBIO, &mode); //Set server to non-blocking
    size_t client_vector_size;
    while (networkServerInfo.isListening) //Continuously accept new clients
    {
        std::this_thread::sleep_for(std::chrono::seconds(1)); //Wait a second before checking again (bot spam protection)
        {//Verify if the server is empty
            std::unique_lock<std::mutex> lock(networkServerLocks.client_vector_mtx);
            client_vector_size = networkServerInfo.client_vector.size();
        }
        SOCKET newClient = accept(networkServerInfo.server, NULL, NULL);
        if (newClient == INVALID_SOCKET)
        {
            std::unique_lock<std::mutex> lock(errors.msg_buffer_mtx);
            int err = WSAGetLastError();
            if (err != WSAEWOULDBLOCK && err != 0) //Take note of the error if it is not a would block call
            {
                errors.msg_buffer = ("Listen error has occured: " + std::to_string(err));
                continue;
            }
            continue; 
        }
        //Accepts new client if possible
        if (client_vector_size < networkServerInfo.MAX_CONNECTIONS)
        {
            ioctlsocket(newClient, FIONBIO, &mode); //Sets newClient to be non-blocking
            std::unique_lock<std::mutex> lock(networkServerLocks.client_vector_mtx);
            networkServerInfo.client_vector.push_back(Network_Server_Client_Vector{newClient, false, "", false}); //Username will come later
        } else
        { //Lock listening until the server is non-empty
            std::unique_lock lock(networkServerLocks.client_vector_capacity_mtx);
            networkServerLocks.isServerFull = true;
            networkServerLocks.client_vector_cv.wait(lock, [&networkServerLocks]{return !networkServerLocks.isServerFull;}); //capture by reference
        }
    }
}

void Network_Server_Broadcast(Network_Error_Msg_Buffer& errors, Network_Server_Locks& networkServerLocks, Network_Server_Information& networkServerInfo)
{
    int byteCount;
    networkServerInfo.isBroadcasting = true;
    while (1)
    {
        {
            std::unique_lock<std::mutex> lock(networkServerLocks.sending_data_mtx);
            networkServerLocks.sending_data_cv.wait(lock);
        }
        if (!networkServerInfo.isBroadcasting) break; //Exit thread
        std::scoped_lock broadcast_lock(networkServerLocks.client_vector_mtx, networkServerLocks.client_vector_capacity_mtx);
        if (networkServerLocks.isServerEmpty)
        {
            std::unique_lock<std::mutex> lock(errors.msg_buffer_mtx);
            errors.msg_buffer = ("Message failed to send as server is empty");
        }
        for (int i = 0; i < networkServerInfo.client_vector.size(); i++)
        {
            SOCKET clientSock = networkServerInfo.client_vector[i].socket;
            byteCount = send(clientSock, (char*)&networkServerInfo.dataToSend, sizeof(networkServerInfo.dataToSend), 0);
            if (byteCount == SOCKET_ERROR)
            {
                std::scoped_lock broadcast_lock(networkServerLocks.client_vector_mtx, networkServerLocks.client_vector_capacity_mtx);
                errors.msg_buffer = ("Message failed to send to " + networkServerInfo.client_vector[i].assignedUsername);
                continue;
            }
            if (byteCount == 0)
            {
                std::scoped_lock broadcast_lock(networkServerLocks.client_vector_mtx, networkServerLocks.client_vector_capacity_mtx);
                errors.msg_buffer = (networkServerInfo.client_vector[i].assignedUsername + " has disconnected. Send Failed");
            }
        }
    }

}

void Network_Server_MirrorMessage(const ChatObject& data, Network_Error_Msg_Buffer& errors, Network_Server_Locks& networkServerLocks, Network_Server_Information& networkServerInfo)
{
    int byteCount;
    //Scoped lock already initiated in Receive
    for (int i = 0; i < networkServerInfo.client_vector.size(); i++)
    {
        if (!networkServerInfo.client_vector[i].didSendData) //Mirror message to all the other sockets
        {
            SOCKET clientSock = networkServerInfo.client_vector[i].socket;
            byteCount = send(clientSock, (char*)&data, sizeof(data), 0);
            if (byteCount == SOCKET_ERROR)
            {
                std::scoped_lock broadcast_lock(networkServerLocks.client_vector_mtx, networkServerLocks.client_vector_capacity_mtx);
                errors.msg_buffer = ("Message mirroring failed to send to" + networkServerInfo.client_vector[i].assignedUsername);
            }
        }
    }
}

bool Network_Server_Bind(Network_Error_Msg_Buffer& errors, Network_Server_Information& networkServerInfo)
{
    if (networkServerInfo.dataToSend.getUsername().length() == 0)
    {
        std::unique_lock<std::mutex> lock(errors.msg_buffer_mtx);
        errors.msg_buffer = ("Invalid username");
        return false;
    }
    std::string ip = "0.0.0.0";
    //Bind the socket
    sockaddr_in service;
    service.sin_family = AF_INET;
    InetPtonA(AF_INET, ip.c_str(), &service.sin_addr.S_un);
    service.sin_port = htons(std::stoi(networkServerInfo.port));
    if (bind(networkServerInfo.server, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR)
    {
        closesocket(networkServerInfo.server);
        std::unique_lock<std::mutex> lock(errors.msg_buffer_mtx);
        errors.msg_buffer = ("Socket binding failed");
        return false;
    }
    return true;
}

bool Network_Server_CreateSocket(Network_Server_Information& networkServerInfo)
{
    networkServerInfo.server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //Initialize TCP server
    if (networkServerInfo.server == INVALID_SOCKET) return false;
    return true;
}

void Network_InitializeWSA(Network_Error_Msg_Buffer& errors)
{
    int wsaerr;
    //Desire version 2.2 of winsock2 (most recent as of this code)
    WORD wVersionRequested = MAKEWORD(2,2); 
    wsaerr = WSAStartup(wVersionRequested, &NETWORK_WSADATA);
    if (wsaerr != 0)
    {
        std::unique_lock<std::mutex> lock(errors.msg_buffer_mtx);
        errors.msg_buffer = ("WSA initialization failed.");
    }
}

void Network_Server_Init(int max_allowed_connections, Network_Server_Locks& networkServerLocks, Network_Server_Information& networkServerInfo)
{
    networkServerInfo.isListening = false;
    networkServerInfo.isReceiving = false;
    networkServerLocks.isServerEmpty = true;
    networkServerLocks.isServerFull = false;
    networkServerInfo.MAX_CONNECTIONS = max_allowed_connections;
    networkServerInfo.data_Handler_Func = Network_DefaultHandleData; //Process data
    networkServerInfo.isBroadcasting = false;
    networkServerInfo.dataToSend = ChatObject();
}

bool Network_Server_InitThreads(Network_Error_Msg_Buffer& errors, Network_Server_Locks& networkServerLocks, Network_Server_Information& networkServerInfo)
{
    if (listen(networkServerInfo.server, networkServerInfo.MAX_CONNECTIONS) == SOCKET_ERROR) return false;
    networkServerInfo.serverListeningThread = std::thread(Network_Server_Listen, std::ref(errors), std::ref(networkServerLocks), std::ref(networkServerInfo));
    networkServerInfo.serverReceivingThread = std::thread(Network_Server_Receive, std::ref(errors), std::ref(networkServerLocks), std::ref(networkServerInfo));
    networkServerInfo.serverBroadcastThread = std::thread(Network_Server_Broadcast, std::ref(errors), std::ref(networkServerLocks), std::ref(networkServerInfo));
    return true;
}

void Network_Server_Shutdown(Network_Server_Locks& networkServerLocks, Network_Server_Information& networkServerInfo)
{
    networkServerInfo.isReceiving = false;
    networkServerInfo.isListening = false;
    networkServerInfo.isBroadcasting = false;
    networkServerLocks.isServerEmpty = false; //Needs to be put before notifying receive thread 
    networkServerLocks.isServerFull = false; //Needs to be put before notifying receive thread 
    std::atomic<bool> notifyThread;
    notifyThread = false;
    std::thread notification = std::thread([&notifyThread ,&networkServerLocks]{
        while (!notifyThread)
        {   //Continuously notify to get rid of any race conditions
            networkServerLocks.client_vector_cv.notify_all();
            networkServerLocks.sending_data_cv.notify_all();
        }
    });
    networkServerInfo.serverListeningThread.join();
    networkServerInfo.serverReceivingThread.join();
    networkServerInfo.serverBroadcastThread.join();
    notifyThread = true;
    notification.join(); //Will always join
    shutdown(networkServerInfo.server, SD_BOTH);
    closesocket(networkServerInfo.server);
    networkServerInfo.server = INVALID_SOCKET;
}

bool Network_Client_CreateSocket(Network_Client_Locks& networkClientLocks, Network_Client_Information& networkClientInfo)
{
    networkClientInfo.client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //Initialize TCP server
    if (networkClientInfo.client == INVALID_SOCKET) return false;
    u_long mode = 1;
    {
        std::unique_lock<std::mutex> lock(networkClientLocks.client_mtx);
        ioctlsocket(networkClientInfo.client, FIONBIO, &mode); //Set to non-blocking
    }
    return true;
}

bool Network_Client_ConnectToServer(Network_Error_Msg_Buffer& errors, Network_Client_Locks& networkClientLocks, Network_Client_Information& networkClientInfo) //To be run on another thread while main thread uses loading screen
{
    if (networkClientInfo.dataToSend.getUsername().length() == 0)
    {
        std::unique_lock<std::mutex> lock(errors.msg_buffer_mtx);
        errors.msg_buffer = ("Invalid username");
        return false;
    }

    sockaddr_in clientService; //Connection information
    {
        std::unique_lock<std::mutex> lock(networkClientLocks.client_mtx);
        clientService.sin_family = AF_INET;
        InetPtonA(AF_INET, networkClientInfo.ip.c_str(), &clientService.sin_addr.S_un); //Turn ipv4/ipv6 into standard text representation
        clientService.sin_port = htons(std::stoi(networkClientInfo.port)); //Convert port to network byte order (big-endian)
    }
    fd_set client_check;
    FD_ZERO(&client_check);
    FD_SET(networkClientInfo.client, &client_check); //Add current socket to set
    struct timeval select_wait;
    select_wait.tv_sec = networkClientInfo.CLIENT_CONNECT_TIMEOUT_SEC;
    select_wait.tv_usec = 0;
    int connectRes;
    {
        std::unique_lock<std::mutex> lock(networkClientLocks.winsock_mtx);
        int connectRes = connect(networkClientInfo.client, (SOCKADDR*)&clientService, sizeof(clientService)); //Begin connection attempt
        if (connectRes == 0) return true; //Connection successfull
        connectRes = WSAGetLastError(); //Check error code to determine if it should block
        if (connectRes != WSAEWOULDBLOCK)
        {
            std::unique_lock<std::mutex> lock(errors.msg_buffer_mtx);
            errors.msg_buffer = ("Connection did not block");
            return false;
        }
        connectRes = select(networkClientInfo.client + 1, NULL, &client_check, NULL, &select_wait); //Wait 3 seconds to see if connection is successfull
        if (connectRes <= 0) //Timeout is 0 and anything < 0 means an error with select has occurred
        {
            std::unique_lock<std::mutex> lock(errors.msg_buffer_mtx);
            connectRes == 0 ? errors.msg_buffer = ("Connection Timeout") : errors.msg_buffer = ("Select error");
            return false;
        } 
        socklen_t len = sizeof(connectRes);
        getsockopt(networkClientInfo.client, SOL_SOCKET, SO_ERROR, (char*)&connectRes, &len);
        if (connectRes != 0) //Connection failed
        {
            std::unique_lock<std::mutex> lock(errors.msg_buffer_mtx);
            errors.msg_buffer = ("Connection Failed");
            return false; 
        }
        //If we get here, then the connection was successfull, and it is linked with the server        
    }
    networkClientInfo.isConnected = true;
    return true;
}

void Network_Client_SendToServer(Network_Error_Msg_Buffer& errors, Network_Client_Locks& networkClientLocks, Network_Client_Information& networkClientInfo)
{
    int byteCount; 
    networkClientInfo.isSending = true;
    while (1)
    {
        std::unique_lock<std::mutex> lock(networkClientLocks.client_mtx);
        networkClientLocks.send_data_cv.wait(lock);
        if (!networkClientInfo.isSending) break; //End thread
        byteCount = send(networkClientInfo.client, (char*)&networkClientInfo.dataToSend, sizeof(networkClientInfo.dataToSend), 0);
        if (byteCount == SOCKET_ERROR)
        {;
            std::unique_lock<std::mutex> lock(errors.msg_buffer_mtx);
            if (WSAGetLastError())
            {
                errors.msg_buffer = ("Connection terminated by server");
                continue;
            }
            errors.msg_buffer = ("Message send failed");
            continue;
        }
        if (byteCount == 0)
        {
            std::unique_lock<std::mutex> lock(errors.msg_buffer_mtx);
            errors.msg_buffer = ("Message send failed as server is closed");
        }
    }
}

void Network_Client_ReceiveFromServer(Network_Error_Msg_Buffer& errors, Network_Client_Locks& networkClientLocks, Network_Client_Information& networkClientInfo)
{
    ChatObject dataReceived;
    networkClientInfo.isReceiving = true;
    //We wait 10ms to check if more data will arrive
    fd_set data_check;
    struct timeval select_wait;
    select_wait.tv_sec = 0;
    select_wait.tv_usec = 10000; //Creates waiting (10 ms)
    while (networkClientInfo.isReceiving)
    {
        if (!networkClientInfo.isConnected)
        {
            std::unique_lock lock(networkClientLocks.receive_mtx);
            networkClientLocks.receive_cv.wait(lock);
            if (!networkClientInfo.isReceiving) break; //End thread
        }
        FD_ZERO(&data_check);
        FD_SET(networkClientInfo.client, &data_check);
        int check = select(0, &data_check, NULL, NULL, &select_wait);
        if (check == 0) continue;
        int bytesReceived = recv(networkClientInfo.client, (char*)&dataReceived, sizeof(dataReceived), 0);
        if (bytesReceived == 0)
        { //Disconnected
            std::unique_lock<std::mutex> lock(errors.msg_buffer_mtx);
            errors.msg_buffer = "Connection to server lost";
            networkClientInfo.isConnected = false;
            continue;
        }
        if (bytesReceived == SOCKET_ERROR) //Receive failed for some reason
        {
            std::unique_lock<std::mutex> lock(errors.msg_buffer_mtx);
            int err = WSAGetLastError();
            if (err != WSAEWOULDBLOCK && err != 0 && err != WSAECONNRESET) //Log the error if it is not WSAEWOULDBLOCK
                errors.msg_buffer = ("Connection error has occured: " + std::to_string(err));
            if (err == WSAECONNRESET) networkClientInfo.isConnected = false;
            continue;
        }
        networkClientInfo.data_Handler_Func(dataReceived); //Process data
    }
}

void Network_Client_Init(int client_connect_seconds_timeout, Network_Client_Information& networkClientInfo)
{
    networkClientInfo.isSending = false;
    networkClientInfo.CLIENT_CONNECT_TIMEOUT_SEC = client_connect_seconds_timeout;
    networkClientInfo.dataToSend = ChatObject();
    networkClientInfo.data_Handler_Func = Network_DefaultHandleData;
    networkClientInfo.isReceiving = false;
    networkClientInfo.isConnected = false;
}

void Network_Client_InitThreads(Network_Error_Msg_Buffer& errors, Network_Client_Locks& networkClientLocks, Network_Client_Information& networkClientInfo)
{
    networkClientInfo.clientSendThread = std::thread(Network_Client_SendToServer, std::ref(errors), std::ref(networkClientLocks), std::ref(networkClientInfo));
    networkClientInfo.clientReceiveThread = std::thread(Network_Client_ReceiveFromServer, std::ref(errors), std::ref(networkClientLocks), std::ref(networkClientInfo));
}

void Network_Client_Shutdown(Network_Client_Locks& networkClientLocks, Network_Client_Information& networkClientInfo)
{
    networkClientInfo.isSending = false;
    networkClientInfo.isConnected = false;
    networkClientInfo.isReceiving = false;
    std::atomic<bool> notifyThread;
    notifyThread = false;
    std::thread notification = std::thread([&notifyThread ,&networkClientLocks]{
        while (!notifyThread)
        {   //Continuously notify to get rid of any race conditions
            networkClientLocks.send_data_cv.notify_all();
            networkClientLocks.receive_cv.notify_all();
        }
    });
    networkClientInfo.clientSendThread.join();
    networkClientInfo.clientReceiveThread.join();
    notifyThread = true;
    notification.join(); //Will always work
    shutdown(networkClientInfo.client, SD_BOTH);
    closesocket(networkClientInfo.client); //server handles shutdown()
    networkClientInfo.client = INVALID_SOCKET;
}

void Network_Server_Set_Port(const std::string& port_in, Network_Server_Information& networkServerInfo)
{
    networkServerInfo.port = port_in;
}

void Network_Client_Set_Ip(const std::string& ip_in, Network_Client_Information& networkClientInfo)
{
    networkClientInfo.ip = ip_in;
}

void Network_Client_Set_Port(const std::string& port_in, Network_Client_Information& networkClientInfo)
{
    networkClientInfo.port = port_in;
}

bool Network_Client_LoopbackConnect(Network_Error_Msg_Buffer& errors, Network_Client_Locks& networkClientLocks, Network_Client_Information& networkClientInfo)
{
    networkClientInfo.port = "55555";
    networkClientInfo.ip = "127.0.0.1";
    return Network_Client_ConnectToServer(errors, networkClientLocks, networkClientInfo);
}

void Network_Server_Updates(Network_Server_Locks& networkServerLocks, Network_Server_Information& networkServerInfo)
{
    {
        std::scoped_lock lock(networkServerLocks.client_vector_mtx, networkServerLocks.client_vector_capacity_mtx);
        size_t client_vector_size = networkServerInfo.client_vector.size();
        (client_vector_size == 0) ? networkServerLocks.isServerEmpty = true : networkServerLocks.isServerEmpty = false;
        (client_vector_size >= networkServerInfo.MAX_CONNECTIONS) ? networkServerLocks.isServerFull = true : networkServerLocks.isServerFull = false;
    }
    networkServerLocks.client_vector_cv.notify_all();
}

void Network_Server_SendMsg(Network_Server_Locks& networkServerLocks)
{
    networkServerLocks.sending_data_cv.notify_all();
}

void Network_Client_Updates(Network_Client_Locks& networkClientLocks)
{
    std::lock_guard<std::mutex> lock(networkClientLocks.client_mtx);
    networkClientLocks.send_data_cv.notify_all();
}

bool Network_Server_LoopbackBind(Network_Error_Msg_Buffer& errors, Network_Server_Information& networkServerInfo)
{
    if (networkServerInfo.dataToSend.getUsername().length() == 0)
    {
        std::unique_lock<std::mutex> lock(errors.msg_buffer_mtx);
        errors.msg_buffer = ("Invalid username");
        return false;
    }
    std::string ip = "127.0.0.1";
    networkServerInfo.port = "55555";
    //Bind the socket
    sockaddr_in service;
    service.sin_family = AF_INET;
    InetPtonA(AF_INET, ip.c_str(), &service.sin_addr.S_un);
    service.sin_port = htons(std::stoi(networkServerInfo.port));
    if (bind(networkServerInfo.server, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR)
    {
        closesocket(networkServerInfo.server);
        std::unique_lock<std::mutex> lock(errors.msg_buffer_mtx);
        errors.msg_buffer = ("Socket binding failed");
        return false;
    }
    return true;
}

const std::string Network_GetLastError(Network_Error_Msg_Buffer& errors)
{
    std::lock_guard<std::mutex> lock(errors.msg_buffer_mtx);
    std::string out = errors.msg_buffer;
    errors.msg_buffer.clear();
    return out;
}