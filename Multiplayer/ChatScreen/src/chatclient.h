#ifndef CHATCLIENT_H
#define CHATCLIENT_H

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
#include "chatdata.h"

class ChatClient
{
    private:
        std::string ip;
        int port;
        SOCKET client;
        std::atomic<bool> isRecvRunning;
        std::atomic<bool> isSendRunning;
        std::atomic<bool> isMsgRdyToSend;
        std::thread recvThread;
        std::thread sendThread;
        ChatData dataToSend;

        //void recvFromServer();
        void sendToServer();

    public:
        ChatClient();
        bool connectToServer(std::string ip, int port);
        bool sendMessage(char message[]);
        void disconnect();
        void recvFromServer();
};

#endif