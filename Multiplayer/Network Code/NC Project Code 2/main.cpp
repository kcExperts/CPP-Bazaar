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
        Server server(4, true);
        server.data_Handler_Func = [&server](const int& i, Network_TCP_Data_Obj& data) {
        std::cout << server.get_identifier(i) << ": " << data.getString() << std::endl;
        };
        if (!server.initialize("0", "Test"))
        {
            std::cout << "Error: " << server.get_last_error_msg() << std::endl;
            return 0;
        }
        std::cout << "Server Open" << std::endl;
        int lol = 0;
        while (lol < 50)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << "Error: " << server.get_last_error_msg() << std::endl;
            std::cout << "Size: " << server.getCurrentServerSize() << std::endl;
            lol++;
            if (lol == 10)
            {
                std::cout << "Sending Message" << std::endl;
                server.data.setMessage("Hello!");
                server.dataReadyToSend();
            }
        }
        server.close();
        std::cout << "Server Closed" << std::endl;
    }
    if (code == "c")
    {
        Client client;
        if (!client.initialize("0", "0", "kcExpert", "Test"))
        {
            std::cout << "Error: " << client.get_last_error_msg() << std::endl;
            std::cout << "Connect failed but program good!" << std::endl;
            return 0;
        }
        std::string msg;
        std::cout << "Enter message: ";
        std::getline(std::cin, msg);
        client.data.setMessage(msg);
        client.dataReadyToSend();
        int lol = 0;
        while (lol < 1)
        {
            std::cout << "Error: " << client.get_last_error_msg() << std::endl;
            lol++;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        std::cout << "CLOSING" << std::endl;
        client.close();
    }
    std::cout << "Program Successful" << std::endl;
    return 0;
}