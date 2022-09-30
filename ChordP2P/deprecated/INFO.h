#pragma once
#include <string>
#include <vector>
#include "json.hpp"
#include <stack>

using namespace nlohmann::json_abi_v3_11_2;
typedef struct Node_INFO
{
	long guid;
	std::string ip;
	std::string port;
} Node_INFO;


typedef struct BootNode_INFO
{
	long guid;
	std::string ip;
	std::string port;
}BootNode_INFO;

typedef struct LoadBalancer_INFO
{
	long guid;
	std::string ip;
	std::string port;
}LoadBalancer_INFO;

enum NodeType
{
	_BootNode,
	_LoadBalancer,
	_Client
};


struct Client_INFO
{
	std::string ip;
	std::string port;
	NodeType type;
};


enum class PacketID
{
	LB_INIT,
	CLIENT_INIT,
	CLIENT_INIT_SUCESSORS,
	NEW_CLIENT,
	FIX_FINGER,
	NOTIFY
};

class RequestBuilder
{

public:
	static std::string register_client(Node_INFO info)
	{
		json request = {
			{"id", PacketID::CLIENT_INIT},
			{"ip", info.ip},
			{"port", info.port}
		};
		return request.dump();

	}
	static std::string prepare_client_reponse(std::map<long, std::string> sucessors, long assigned_guid)
	{
		json response = {
			{"id", PacketID::CLIENT_INIT_SUCESSORS},
			{"sucessors", sucessors},
			{"guid", assigned_guid}
		};
		return response.dump();
	}

	static std::string fix_finger() {
		json request = {
			{"id", PacketID::FIX_FINGER}
		};
		return request.dump();
	}

	static std::string send_finger(std::map<long, std::string> fingerTable) {
		json response = {
			{"finger_table", fingerTable}
		};
		return response.dump();
	}
	static std::string notify(std::stack<json> st) {
		{
			std::vector<json> temp(&st.top() + 1 - st.size(), &st.top() + 1);
			json response = {
				{"id", PacketID::NOTIFY},
				{"stack", temp}
			};
			return response.dump();
		}
	}
};