#include "src\ncCodeTCP_T.h"

int main()
{
    #if defined(_WIN32)
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
                std::cout << "Error: " << get_last_error_msg(error_codes) << std::endl;
                return 0;
            }
            std::cout << "Server Open" << std::endl;
            int lol = 0;
            while (lol < 50)
            {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                std::cout << "Error: " << get_last_error_msg(error_codes) << std::endl;
                std::cout << "Size: " << server.get_current_server_size() << std::endl;
                lol++;
                if (lol == 10)
                {
                    std::cout << "Sending Message" << std::endl;
                    server.set_data_to_send("Hello!", 0);
                    server.data_ready_to_send();
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
                std::cout << "Error: " << get_last_error_msg(error_codes) << std::endl;
                std::cout << "Connect failed but program good!" << std::endl;
                return 0;
            }
            std::string msg;
            std::cout << "Enter message: ";
            std::getline(std::cin, msg);
            client.set_data_to_send(msg, 0);
            client.data_ready_to_send();
            int lol = 0;
            while (lol < 1)
            {
                std::cout << "Error: " << get_last_error_msg(error_codes) << std::endl;
                lol++;
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            std::cout << "CLOSING" << std::endl;
            client.close();
        }
        std::cout << "Program Successful" << std::endl;
        return 0;
    #elif defined(__linux__)

    #endif
}