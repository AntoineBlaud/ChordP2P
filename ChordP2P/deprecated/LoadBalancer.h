#pragma once

#include <vector>

#include "Node.h"
#include "BootNode.h"
#include "Socket.h"

#include "INFO.h"
#include "json.hpp"
#include "random"
#include "utils.h"
#include "Periodic.h"

using namespace nlohmann::json_abi_v3_11_2;

class LoadBalancer
{

private:	

	Socket socket;
	LoadBalancer_INFO lb_info;
	vector<BootNode_INFO> bn_networks;

public:

	LoadBalancer(LoadBalancer_INFO lb_info);
	~LoadBalancer(void);

	// copy constructor
	LoadBalancer(const LoadBalancer& other);


	// copy assignment operator
	LoadBalancer& operator=(const LoadBalancer& other);
	

	void Join(Socket * client);
	void registerBootNode(json info, Socket * client);
	void registerNode(json info, Socket  * client);
	void start(void);

};

void receiveData(Socket* client, std::string& buffer);
