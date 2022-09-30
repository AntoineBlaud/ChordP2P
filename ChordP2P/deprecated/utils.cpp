#include "utils.h"

void receiveData(Socket* client, std::string& buffer)
{
    while (true) {
        vector<Socket> reads(1);
        reads[0] = *client;
        int seconds = 10; //Wait 10 seconds for input
        if (Socket::select(&reads, NULL, NULL, seconds) < 1) { //Socket::select waits until masterSocket reveives some input (for example a message)
            //No Input
            continue;
        }
        else {
            client->socket_read(buffer, 1024); //Read 1024 bytes of the stream
            break;
        }
    }
}


void log_text_periodic(std::string const& text)
{
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::cout << text << std::endl;
}
