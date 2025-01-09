#include "src\ncCodeTCP.h"

int main()
{
    std::thread err_representation;
    if (!Network_InitializeWSA())
    {
        std::cout << "WSA Initialization Failed" << std::endl;
        return 0;
    }
    std::cout << "Enter \"s\" for server or \"c\" if client: ";
    std::string code;
    std::getline(std::cin, code);
    if (code == "s")
    {
        Server server(4);
        if (!server.initialize("0"))
        {
            std::cout << "Error: " << server.getLastErrorMsg() << std::endl;
            return 0;
        }
        std::cout << "Server Open" << std::endl;
        int lol = 0;
        while (lol < 10)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << "Error: " << server.getLastErrorMsg() << std::endl;
            lol++;
        }
        server.close();
        std::cout << "Server Closed" << std::endl;
    }
    if (code == "c")
    {
        Client client;
        if (!client.initialize("0", "0"))
        {
            std::cout << "Error: " << client.getLastErrorMsg() << std::endl;
            return 0;
        }
        std::string msg;
        std::cout << "Enter message: ";
        std::getline(std::cin, msg);
        client.data.info.setMessage(msg);
        client.dataReadyToSend();
        int lol = 0;
        while (lol < 1)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << "Error: " << client.getLastErrorMsg() << std::endl;
            lol++;
        }
        client.close();
    }
    std::cout << "Program Successful" << std::endl;
    return 0;
}