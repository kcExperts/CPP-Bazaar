#include <winsock2.h>
#include <ws2tcpip.h> //For InetPton function
#include <tchar.h> //For _T macro
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>

//Requires WSA data to be initialized

class ChatServer
{
    private:
        std::string ip; 
        int port;
        int maxConnections;
        int maxAllowedChars;
        SOCKET server;
        std::vector<SOCKET> serverConnections;
        std::atomic<bool> running;
        std::thread listenThread;

        //Stops the listening thread
        void stopThread();
        //Receives data
        void recvFromClient(SOCKET client, int clientID);

        int totalConnections;
    public:
        //Handles WSA initialization, sohuld only be called once
        ChatServer(); 
        //Creates and binds a socket to the given ip and port, specifying the max connections and the size of the recvBuffer
        bool create(std::string ip, int port, int maxConnections, int recvBuffer);
        //Sets the server to listening mode, accepting new clients when possible
        void listenForClients();
        //Forces the server to stop accepting new clients
        void stopListening();
        //Shutsdown a given server
        void shutItDown();




        

};
