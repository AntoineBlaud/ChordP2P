#pragma once
#include <vector>
#include <map>
#include <string>
#include <stack>
#include <iostream>
#include <chrono>
#include <thread>
#include <functional>

#include "INFO.h"
#include "json.hpp"
#include "Socket.h"
#include "utils.h"
#include "Periodic.h"

using namespace nlohmann::json_abi_v3_11_2;

class BootNode
{
public:
	BootNode(BootNode_INFO bn_info);
	~BootNode(void);
	
	// copy constructor
	BootNode(const BootNode& other);

	BootNode& operator=(BootNode&& other);

	int Register(Socket* client, json info);
	int Unregister(Socket* client, json info);
	int RescueNode(Socket* client, json info);
	int Join(LoadBalancer_INFO lb_info);
	void start(void);
	void Notify();
	
private:
	long guid;
	long RING_SIZE = 128;
	long RING_START = 0;
	BootNode_INFO bn_info;
	Socket srvSocket;
	Socket clientSocket;
	
	// List of availbale BootNode
	std::vector<long> bootNodeTable;
	// Key:File ID, Value: List of GUID
	std::map <long, std::vector<long>> superFileTable;
	// Key: GUID, Value: IP
	std::map<long, std::string> superGUIDTable;
	std::stack <json> messagePool;

	// Collect Node query, and send to BootNode when it is full
	int TransferBlockQuery();

	// update the superFileTable and superGUIDTable
	int UpdateSuperTable(); 
	
	void HandleIncommingRequest(Socket * client);
	
};
