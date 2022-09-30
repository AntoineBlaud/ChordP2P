#pragma once
#include "Socket.h"
#include <boost/noncopyable.hpp>
#include "common.hpp"
#include "definitions.h"



template<class usock, class child>
class TcpCommunicator
{
public:
    TcpCommunicator(TCP_ADDR  tc, child * ch) { 
        this->childClass = ch; 
        this->tcpCom = tc; 
        this->StartBackgroundJobs();
        this->workers = std::vector<std::thread>(); };
    virtual ~TcpCommunicator() = default;
	
	//TcpCommunicator& operator=(const TcpCommunicator& other)
    TcpCommunicator(const TcpCommunicator& other) {
        this->tcpCom = other.tcpCom;
        this->childClass = other.childClass;
    }

	void StartListening();

    void StartBackgroundJobs();

    std::shared_ptr<usock>  srvSocket;
	
protected:
	child* childClass;
	TCP_ADDR tcpCom;
    std::vector<std::thread> workers;

};


template<class usock, class child>
inline void TcpCommunicator<usock, child>::StartListening()
{
    cout << "Start listening" << endl;
    this->srvSocket = std::make_shared<usock>(usock(AF_INET, SOCK_STREAM, 0));
    cout << this->tcpCom.srv_ip << ":" << this->tcpCom.srv_port << endl;
    this->srvSocket->bind(this->tcpCom.srv_ip, this->tcpCom.srv_port);
    this->srvSocket->listen(1000);
    while (true) {
        try {
            vector<usock*> reads(1);
            reads[0] = this->srvSocket.get();
            int seconds = 1; //Wait 20 seconds for incoming Connections
            if (usock::select(&reads, NULL, NULL, seconds) < 1) { //Socket::select waits until masterSocket reveives some input (for example a new connection)
                //No new Connection
                continue;
            }
            else {
                //Something happens, let's accept the connection
                 //Something happens, let's accept the connection
                //usock* client = (this->srvSocket->accept());
                //child * child_handle = this->childClass;
                //std::async(std::launch::async, [&child_handle, client] { child_handle->HandleIncommingRequest(*client); });


                usock* client = (this->srvSocket->accept());
                this->childClass->HandleIncommingRequest(*client);
                client->socket_shutdown(2);
                client->_close();
            }
        }catch (...) {
            cout << "Socket server error " << endl;
        }
    }
	
}



template<class usock, class child>
void TcpCommunicator<usock, child>::StartBackgroundJobs()
{
	std::thread t1(&TcpCommunicator<usock, child>::StartListening, std::move(static_cast<TcpCommunicator<usock, child>>(*this)));
    t1.detach();
    this->workers.push_back(std::move(t1));
	
}