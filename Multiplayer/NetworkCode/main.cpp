#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h> //InetPtonA function

#include <iostream>
#include <string>

#include <thread>
#include <atomic>
#include <future>
#include <condition_variable>
#include <vector>
#include <chrono> //Used to make thread wait (for non-blocking sockets)

#include "ChatObject.h"

//Required to initialize winsock2 and the sockets
namespace { //Ensures these cannot be seen outside of the file
WSAData wsaData;

//Connection information
std::string port;
std::string ip;

//Sockets
SOCKET server;
SOCKET client;
}

enum Network_Basic_Errors
{
    None,
    Initialization,
    ServerSocketCreation,
    ServerSocketBinding,
    ListeningInitialization,
    UserFailedToConnect
};

enum Network_Receive_Information
{
    ReceiveSuccess,
    ReceiveFailure,
    Disconnected
};

Network_Basic_Errors Network_InitializeWSA();
Network_Basic_Errors Network_Server_CreateSocket();
Network_Basic_Errors Network_Server_Bind();

//Listening
void Network_Server_Listen();
std::thread serverListeningThread;
std::atomic<bool> isListening;
std::mutex listening_mutex; //Mutex to verify if connections work
std::mutex capacity_mutex; //Mutex for checking if we are at max capacity
std::condition_variable listeningCondition;
bool userConnectWorking; //Need not be an atomic as listening halts until it is set to true
bool isServerFull; //Used for halting accepting
bool isServerEmpty; //Used to halt receiving thread
std::vector<SOCKET> serverClients;
#define MAX_CONNECTIONS 4
std::promise<Network_Basic_Errors> listeningErrorDetection_p; //Promises to return a value at some point
//If we try and get this value before thread ends, program will block until it is ready to go again
auto listeningErrorDetection_f = listeningErrorDetection_p.get_future(); //Will receive the promised value (need not be global)


//Handling data thread
void Network_HandleData();
std::thread handleDataThread;
std::atomic<bool> canHandleData;
std::mutex handlingData_mutex;
std::condition_variable handlingDataCondition;

//Receiving
void Network_Server_Receive();
std::thread serverReceivingThread;
std::atomic<bool> isReceiving;
bool isDataProcessed;
std::mutex receiving_mutex;
std::condition_variable receivingCondition;
std::promise<const ChatObject&> dataToHandle_p;
auto dataToHandle_f = dataToHandle_p.get_future(); //May not need global


int main(void)
{
    isDataProcessed = true; //Needs to be like so
    canHandleData = true; //Should only be set to false at the end of the program
    bool isServerEmpty = true;
    Network_Basic_Errors errorDetection;
    errorDetection = Network_InitializeWSA();
    if (errorDetection != None) return errorDetection;
    //Determine is the socket is to be a client or a server
    std::cout << "Enter \"s\" for server or \"c\" if client: ";
    std::string code;
    std::getline(std::cin, code);
    std::cout << std::endl;
    if (code == "s") errorDetection = Network_Server_CreateSocket();
    if (code == "c") //Create client
    if (errorDetection != None) return errorDetection;

    //Bind appropriate socket
    if (code == "s") errorDetection = Network_Server_Bind();
    if (code == "c") //Create client
    if (errorDetection != None) return errorDetection;

    if (code == "s")
    {
        //Spawn Listening Thread
        serverListeningThread = std::thread(Network_Server_Listen);
        //Spawn Handling Thread (needs to be done before receiving)
        handleDataThread = std::thread(Network_HandleData);
        //Spawn Receiving Thread
        serverReceivingThread = std::thread(Network_Server_Receive);
        //End program after 100 sec
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    } else
    {
        //Client shit
    }
    std::cout << "HERE0" << std::endl;
    isListening = false;
    serverListeningThread.join();
    std::cout << "HERE1" << std::endl; //DOES NOT REACH, PROBLEM IN LINES 221 to 231
    isReceiving = false;
    serverReceivingThread.join();
    std::cout << "HERE2" << std::endl;
    canHandleData = false;
    handleDataThread.join();
    std::cout << "HERE3" << std::endl;

    return 0;
}

struct ChatState {
    std::mutex mtx; //Anytime I do anything, lock
    bool isServerGay;
    int numCLients;
    std::vector<Client> clients;
};

/*
Checks every socket to see if a message has been sent, halting to process the data once received.
It is assumed that receiving takes no longer than 10 millisecond.
****MAY NEED TO CHECK FOR BLOCKING CONDITIONS (I.e the case where the client sent nothing, what happens?)
*/
void Network_Server_Receive()
{

    ChatObject dataReceived;
    isReceiving = true;
    fd_set socketReadCheck;
    struct timeval selectWait;
    selectWait.tv_sec = 0;
    selectWait.tv_usec = 10000;

    while (isReceiving)
    {
        std::unique_lock lock(receiving_mutex);
        receivingCondition.wait(lock, []{return !isServerEmpty;});
        
        for (int i = 0; i < serverClients.size(); i++)
        {
            size_t serverClientsSize = serverClients.size(); //Used for communication with Listen Thread
            SOCKET client = serverClients.at(i);
            FD_ZERO(&socketReadCheck); //Clear previous socket from the set, we only want to check the current one
            FD_SET(client, &socketReadCheck); //Add current socket to set
            int check = select(0, &socketReadCheck, NULL, NULL, &selectWait);
            //verify that check = 0 to continue, if not, it should say that receiving message failed cause timeout and continue to next socket
            int bytesReceived = recv(client, (char*)&dataReceived, sizeof(dataReceived), 0);
            if (bytesReceived == 0) //Client has disconnected
            {
                std::cout << "Client Disconnected" << std::endl;
                shutdown(client, SD_BOTH);
                closesocket(client);
                serverClients.erase(serverClients.begin() + i);
                i--; //Readjust to ensure next socket is not skipped
                { //Nerw scope, lock obliterated when out of scope, effectively unlocking mtx
                    std::lock_guard lock(capacity_mutex); //Avoids the assignment below from being discarded
                    if (serverClientsSize >= MAX_CONNECTIONS) isServerFull = false; //Tell listening thread to continue
                }
                continue;
            }
            if (bytesReceived == SOCKET_ERROR) //Receiving failed for some reason
            {
                std::cout << "Received Failed" << std::endl;
                continue;
            }
            //Halt thread until socket has its data processed
            dataToHandle_p.set_value(dataReceived);
            isDataProcessed = false;
            std::unique_lock lock(receiving_mutex);
            receivingCondition.wait(lock, []{return isDataProcessed;});
        }
    }
}

void Network_HandleData()
{
    canHandleData = true;
    ChatObject data;
    while(canHandleData)
    {
        std::unique_lock lock(handlingData_mutex);
        handlingDataCondition.wait(lock, []{return !isDataProcessed;});
        if (!canHandleData) break;
        data = dataToHandle_f.get();
        std::cout << data.getMessage() << std::endl;
    }
}

void Network_Server_Listen()
{
    //Set the server socket to non-blocking
    u_long mode = 1; //Non-blocking mode
    ioctlsocket(server, FIONBIO, &mode);
    userConnectWorking = true;

    isListening = true; 
    if (listen(server, MAX_CONNECTIONS) == SOCKET_ERROR)
    {
        listeningErrorDetection_p.set_value(ListeningInitialization);
        isListening = false;
    }
    while (isListening) //Continuously accept new clients, should only be halted at end of program or if desired
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); //Allows time for clients to connect
        SOCKET newClient = accept(server, NULL, NULL); //Checks a queue to see if any clients are present
        if (newClient == INVALID_SOCKET)
        {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
            {
                //Halt thread until main program detects that someone failed to connect, but do not stop listening
                listeningErrorDetection_p.set_value(UserFailedToConnect);
                userConnectWorking = false;
                //Wait until thread allows listening to continue (during listening halting, will need to be updated)
                std::unique_lock lock(listening_mutex);
                //Wait until cv is notified and that userConnectWorking is true
                listeningCondition.wait(lock, []{return userConnectWorking;}); 
            }
            continue; //Wait for a new client to appear
        }
        //Accepts new client if possible
        if (serverClients.size() < MAX_CONNECTIONS)
        {
            ioctlsocket(newClient, FIONBIO, &mode); //Sets newClient to be non-blocking
            
            serverClients.push_back(newClient);
        } else
        {
            //If max connections reach, halt listening until there is room
            isServerFull = true;
            std::unique_lock lock(capacity_mutex);
            listeningCondition.wait(lock, []{return !isServerFull;});
        }
        isServerEmpty = serverClients.size() == 0; //Ensure that receiving thread is sleeping
    }
}

Network_Basic_Errors Network_Server_Bind()
{
    //Get usert input
    std::cout << "Enter 5-digit port number: ";
    std::getline(std::cin, port);
    std::cout << std::endl;
    std::cout << "Enter 0 for Port forward or nothing for feedback Ip: ";
    std::string detection;
    std::getline(std::cin, detection);
    if (detection == "") ip = "127.0.0.1";
    if (detection == "0") ip = "0.0.0.0";
    //Bind the socket
    sockaddr_in service;
    service.sin_family = AF_INET;
    InetPtonA(AF_INET, ip.c_str(), &service.sin_addr.S_un);
    service.sin_port = htons(std::stoi(port));
    if (bind(server, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR)
    {
        closesocket(server);
        return ServerSocketBinding;
    }
    return None;
}

Network_Basic_Errors Network_Server_CreateSocket()
{
    //Initialize TCP server
    server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server == INVALID_SOCKET) return ServerSocketCreation;
    return None;
}

Network_Basic_Errors Network_InitializeWSA()
{
    int wsaerr;
    //Desire version 2.2 of winsock2 (most recent as of this code)
    WORD wVersionRequested = MAKEWORD(2,2); 
    wsaerr = WSAStartup(wVersionRequested, &wsaData);
    if (wsaerr != 0) return Initialization;
    return None;
}

