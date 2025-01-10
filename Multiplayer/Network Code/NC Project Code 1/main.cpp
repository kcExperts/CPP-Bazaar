#include "networkcodeTCP.h"

int main(void)
{
    Network_Error_Msg_Buffer gen_err;
    Network_Server_Locks server_locks;
    Network_Server_Information server_info;
    Network_Client_Locks client_locks;
    Network_Client_Information client_info;
    Network_InitializeWSA(gen_err);
    std::cout << "Enter \"s\" for server or \"c\" if client: ";
    std::string code;
    std::getline(std::cin, code);
    std::cout << std::endl;
    if (code == "s") 
    {
        std::cout << "Server" << std::endl;
        Network_Server_Init(4, server_locks, server_info);
        server_info.dataToSend.setUsername("Server");
        std::cout << server_info.dataToSend.getMessage() << std::endl;
        if (!Network_Server_CreateSocket(server_info)) return 1;
        if (!Network_Server_LoopbackBind(gen_err, server_info)) return 1;
        if (!Network_Server_InitThreads(gen_err, server_locks, server_info)) return 1;
        int lol = 0;
        //for (;;)
        while(lol < 10)
        {
            lol++;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            Network_Server_Updates(server_locks, server_info);

            std::unique_lock<std::mutex> lockB(server_locks.client_vector_capacity_mtx);
            std::cout << "Server Size: " << server_info.client_vector.size() << std::endl;
            std::cout << Network_GetLastError(gen_err) << std::endl;
            if (lol == 5)
            {
                server_info.dataToSend.setMessage("Hello");
                Network_Server_SendMsg(server_locks);
            }
        }
        Network_Server_Shutdown(server_locks, server_info);
    }
    if (code == "c")
    {
        Network_Client_Init(5, client_info);
        client_info.dataToSend.setUsername("Client");
        if (!Network_Client_CreateSocket(client_locks, client_info)) return 1;
        if (!Network_Client_LoopbackConnect(gen_err, client_locks, client_info)) return 1;
        Network_Client_InitThreads(gen_err, client_locks, client_info);
        //No errors before
        std::string msg;
        std::cout << "Enter message: ";
        std::getline(std::cin, msg);
        client_info.dataToSend.setMessage(msg);
        Network_Client_Updates(client_locks);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        Network_Client_Shutdown(client_locks, client_info);
        std::cout << Network_GetLastError(gen_err) << std::endl;
    }
    std::cout << "Program Complete" << std::endl;
    return 0;
}