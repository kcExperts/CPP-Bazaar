#ifndef WINDOWSERVER_H
#define WINDOWSERVER_H

/*
Add way to check if message was broadcasted to client
May need stopListening function (or may not as it listens up to maxConnections it appears)
*/

#include "precomp.h"
#include <tchar.h> //For _T macro
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <unordered_map>
#include <memory>
#include "windowdata.h"
#include "chatscreen.h"

class WindowServer
{
    private:
        std::string ip;
        int port;
        int maxConnections;
        bool failure;
        int totalConnections;
        int ID;
        ChatScreen* chat;
        SOCKET server;
        std::atomic<bool> isListenRunning;
        std::thread listenThread;
        //Holds client ID and information for halting client specific thread
        std::unordered_map<int, std::pair<std::unique_ptr<std::atomic<bool>>, SOCKET>> clientConnectionFlags; 
    public:
        WindowServer();
        ~WindowServer();
        bool create(std::string ip, int port, int maxConnections, ChatScreen& chatRef);
        void listenForClients();
        void broadcast(WindowData& data);
        bool isWorking();
        void shutDown();
    private:
        void receiveFromClient(int ID);
};

#endif