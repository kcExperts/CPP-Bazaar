#ifndef WINDOWCLIENT_H
#define WINDOWCLIENT_H

/*
TODO:
    - Fix sendMessage (its a pile of shit)
    - CHANGE sentToServer to not be on a while loop
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
#include <chrono>

class WindowClient
{
    private:
        std::string ip;
        int port;
        bool failure;
        std::atomic<bool> newData;
        WindowData data;
        WindowData dataToSend;
        
        SOCKET client;
        std::thread recvThread;
        std::atomic<bool> isRecvRunning;

        std::thread sendThread;
        std::atomic<bool> isSendRunning;
        std::atomic<bool> isMsgRdyToSend;

    public:
        WindowClient();
        ~WindowClient();
        bool connectToServer(std::string ip, int port);
        void receiveFromServer();
        bool isWorking();
        bool isThereNewData();
        void shutDown();
        void sendMessage(const char message[]);
        const WindowData& getData(); 
    private:
        void sendToServer();
};

#endif