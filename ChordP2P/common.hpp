#pragma once
#include <string>
#include <ctime>
#include <chrono>
#include <thread>
#include <cstdio>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <random>
#include "definitions.h"

#ifndef COMMON_H
#define COMMON_h



template <class T>
auto log_text_periodic(T&& text)
{
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::cout << text << std::endl;
}
template <class T>
auto helper_receive_data(T  client, std::string& buffer)
{
    while (true) {
        std::vector<T> reads(1);
        reads[0] = client;
        int seconds = 10; //Wait 10 seconds for input
        if (T::select(&reads, NULL, NULL, seconds) < 1) { //Socket::select waits until masterSocket reveives some input (for example a message)
            //No Input
            continue;
        }
        else {
            client.socket_read(buffer, MAX_READ_SIZE);
            break;
        }
    }
}


class RequestBuilder
{
public:
	static std::string JoinChord(CHORD_INFO chInfos, NODE_INFO ndInfos) {
		json data = {
			{"IP", ndInfos.tcpinfos.srv_ip},
			{"PORT", ndInfos.tcpinfos.srv_port},
			{"PACKET_ID", PacketID::CLIENT_INIT}
		};
		return data.dump();
	}
	
	template <typename T1, typename T2>
	static std::string PrepareClientReponse(T1 sucessors, T2 assigned_guid)
	{
		json data = {
			{"PACKET_ID", PacketID::CLIENT_INIT_RESPONSE},
			{"SUCCESSORS", sucessors},
			{"GUID", assigned_guid}
		};
		return data.dump();
	}
	
	static std::string PrepareFixFinger() {
		json data = {
			{"PACKET_ID", PacketID::FIX_FINGER}
		};
		return data.dump();
	}
	
	template <typename T1>
	static std::string PrepareSendFinger(T1 & fingerTable) {
		json data = {
			{"FINGER_TABLE", fingerTable}
		};
		return data.dump();
	}
	template <typename T1>
	static std::string PrepareNotify(T1 & new_nodes)
	{
		json data = {
			{"PACKET_ID", PacketID::NOTIFY},
			{"NEW_NODES", new_nodes},
		};
		return data.dump();
	}

};



#endif // !COMMON_H


