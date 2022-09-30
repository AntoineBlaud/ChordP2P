#pragma once

#include <string>
#include <vector>
#include <map>
#include <string>
#include <stack>
#include <iostream>
#include <chrono>
#include <thread>
#include <functional>

#include "json.hpp"

#define RING_SIZE 1024
#define MAX_READ_SIZE 4096


using namespace nlohmann::json_abi_v3_11_2;

using UindexTbNode = std::map<long, std::string>;
using UindexTbChord = std::map<long, std::string>;
using UrangeChord = std::pair<long, long>;


typedef struct 
{
	std::string srv_ip;
	std::string srv_port;
}TCP_ADDR;

typedef struct
{
	long guid;
	UrangeChord range;
	TCP_ADDR tcpinfos;
}CHORD_INFO;

typedef struct
{
	long guid;
	TCP_ADDR tcpinfos;
}NODE_INFO;

enum class PacketID
{
	CLIENT_INIT,
	CLIENT_INIT_RESPONSE,
	CLIENT_JOINED,
	FIX_FINGER,
	NOTIFY
};
