#ifndef NCCODETCP_H
#define NCCODETCP_H

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mutex>
#include <condition_variable>

//Intialize WSA
void Network_InitializeWSA();
//Server --
class Server
{
    public:
        
    private:

//Create socket
//Bind socket
//Connect
//Listen
//Receive
//Send
//Broadcast (all or except to)
//Shutdown
//Copy socket function and clients
};

class Client
{
//Create socket
//Connect
//Receive
//Send
//Shutdown
};


#endif