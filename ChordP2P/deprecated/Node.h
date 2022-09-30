#pragma once
#include <vector>
#include <map>
#include <string>
#include <sys/socket.h>
#include "Socket.h"
#include "INFO.h"
#include <fstream>
#include "json.hpp"
#include "utils.h"
#include "Periodic.h"

using namespace nlohmann::json_abi_v3_11_2;


class Node
{
private:
	BootNode_INFO masterBootNode;
	Node_INFO nd_info;
	std::map<long, std::string> fingerTable;
	long predecessorGUID;
	std::int16_t bindPort;
	Socket srvSocket;
	Socket clientSocket;

	void Stabilize();
	void Register(json);
	void Register(BootNode_INFO);
	void leaves();
	void FindSucessor();
	void SendFinger(Socket  * client);
	void Notify(json info);


public:
	Node(Node_INFO nd_info);
	~Node(void);

	// copy constructor
	Node(const Node& other);

	// copy assignment operator
	Node& operator=(const Node&);

	void Join(LoadBalancer_INFO);
	void FixFingers();
	void start(void);
	void HandleIncommingRequest(Socket*);
};
	
	