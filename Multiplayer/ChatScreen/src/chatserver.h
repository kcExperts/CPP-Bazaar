#include <winsock2.h>
#include <ws2tcpip.h> //For InetPton function
#include <tchar.h> //For _T macro
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <unordered_map>
#include <memory>

//Requires WSA data to be initialized

class ChatServer
{
    private:
        std::string ip; 
        int port;
        int maxConnections;
        int maxAllowedChars;
        SOCKET server;
        std::atomic<bool> running;
        std::thread listenThread;
        //Holds client ID and information for halting client specific thread
        std::unordered_map<int, std::pair<std::unique_ptr<std::atomic<bool>>, SOCKET>> clientConnectionFlags;


        //Stops the listening thread
        void stopThread();
        //Receives data
        void recvFromClient(int clientID);

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
        //Shutsdown a given clients thread
        void stopClient(int clientID);
        //Shutsdown a given server
        void shutItDown();
        //Send a message to the client
        void sendToClient(int clientID, const std::string& message);
        //Send to all clients
        void broadcast(const std::string& message);




        

};
