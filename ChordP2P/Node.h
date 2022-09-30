#pragma once


#include "json.hpp"
#include "Periodic.h"
#include "TcpCommunicator.hpp"
#include "definitions.h"
#include "common.hpp"
#include <stdexcept>
#include <exception>

using namespace nlohmann::json_abi_v3_11_2;

#define DEBUG 

using usock = Socket;
#define CLASS_TEMPLATE Node<usock>
template<class usock>
class Node
{
	using TcpComTemplate = TcpCommunicator<usock, CLASS_TEMPLATE>;
protected:
	UindexTbNode fingerTable;
	UindexTbNode notifyCacheTable;
	NODE_INFO ndInfos;
	CHORD_INFO chInfos;
	long predecessorGUID;
	PeriodicScheduler * scheduler;
	std::shared_ptr<TcpComTemplate> tcpCom;

	
	void Stabilize();

	void leaves();
	
	template <class J>
	void Notify(J&);

	void SendFinger(usock& client);

	
	
public:
	Node(NODE_INFO ni) { 
		this->ndInfos = ni; 
		tcpCom = std::make_shared<TcpComTemplate>(TcpComTemplate(ni.tcpinfos, this));
	};
	~Node() {};
	Node& operator=(const Node& other) {
		this->ndInfos = other.ndInfos;
		this->scheduler = other.scheduler;
	}
	Node(const Node& other) = default;
	void Register(CHORD_INFO chs, PeriodicScheduler& periodic_sheduler);
	
	void HandleIncommingRequest(usock &);

	friend void schedule_fingers(PeriodicScheduler& scheduler, CLASS_TEMPLATE& n);

	void FixFingers();

	void StartScheduleJobs(PeriodicScheduler&);
	std::vector<std::thread> workers;


private:
	template <class J>
	void InitFingerTable(J&);
};

template<class usock>
template <class J>
void CLASS_TEMPLATE::InitFingerTable(J& infos)
{
	map<long, std::string> successors = infos["SUCCESSORS"];

	for (auto& item : successors) {
		this->fingerTable[item.first] = std::string(item.second);
	}
}


template<class usock>
void CLASS_TEMPLATE::Stabilize()
{
}


template<class usock>
void CLASS_TEMPLATE::leaves()
{
}

template<class usock>
void CLASS_TEMPLATE::SendFinger(usock& client)
{
	try {
		client.socket_write(RequestBuilder::PrepareSendFinger(this->fingerTable));
		client.socket_shutdown(2);
		client._close();
	}catch (...) {
		cout << "Error sending finger ... " << endl;
	}
}
template<class usock>
void CLASS_TEMPLATE::HandleIncommingRequest(usock& client)
{
	std::string buffer;
	helper_receive_data<usock>(client, buffer);
	json data = json::parse(buffer);
	PacketID id = data["PACKET_ID"];
	switch (id)
	{
	case PacketID::FIX_FINGER:
		this->SendFinger(client);
		break;
	
	case PacketID::NOTIFY:
		this->Notify(data);
		break;

	default:
		break;
	}
	client.socket_shutdown(2);
	client._close();
}

template <class usock>
void CLASS_TEMPLATE::Register(CHORD_INFO chs, PeriodicScheduler& periodic_sheduler)
{
	// send the request
	try {
		this->chInfos = chs;
		this->scheduler = &periodic_sheduler;
		auto sock = std::make_unique<usock>(usock(AF_INET, SOCK_STREAM, 0));
		sock->connect(this->chInfos.tcpinfos.srv_ip, this->chInfos.tcpinfos.srv_port);
		std::string request = RequestBuilder::JoinChord(this->chInfos, this->ndInfos);
		sock->socket_write(request);
		// receive the response
		std::string response;
		helper_receive_data<usock>(*sock, response);
		json data = json::parse(response);
		sock->socket_shutdown(2);
		sock->_close();
		this->ndInfos.guid = data["GUID"];
		this->InitFingerTable<json>(data);
		this->StartScheduleJobs(*this->scheduler);
	}
	catch (...) {
		cout << "Error while registring ... ";
	}
	
}

template<class usock>
template<class J>
void CLASS_TEMPLATE::Notify(J& infos)
{
	cout << "Node " << this->ndInfos.guid << " notified" << endl;
	std::vector<std::string> new_nodes = infos["NEW_NODES"];
	for (auto& node : new_nodes) {
#ifdef DEBUG
		cout << node << endl;
#endif
		json node_parsed = json::parse(node);
		long guid  = node_parsed["GUID"];
		this->notifyCacheTable[guid] = node;
	}
}

template<class usock>
void CLASS_TEMPLATE::FixFingers()
{
#ifdef DEBUG
	cout << "Fixing fingers called from client :" << this->ndInfos.guid << endl;
#endif
	int next = this->ndInfos.guid;
	vector<long> guids = vector<long>(log2(RING_SIZE) + 1);
	for (int i = 0; i < guids.size(); i++)
		guids[i] = next + pow(2, i);

	// temporary finger cache with all the finger table from the node that responds
	std::map<long, std::string> finger_cache = std::map<long, std::string>();
	
	// For each guid in the finger table
	for (int i = 0; i < log2(RING_SIZE); i++)
	{
		for (auto& item : this->fingerTable)
		{
			// Query the good node
			if (!(item.first >= guids[i] && item.first <= guids[i + 1]))
				continue;
			json sucessor_info = json::parse(item.second);
			string ip = sucessor_info["IP"];
			string port = sucessor_info["PORT"];
			
			try {
				// Recreate socket
				auto sock = std::make_unique<usock>(usock(AF_INET, SOCK_STREAM, 0));
				sock->connect(ip, port);
				sock->socket_write(RequestBuilder::PrepareFixFinger());
				string response;
				helper_receive_data(*sock, response);
				cout << "Fetching finger table from: "<< item.first  << endl;
				// ici faire un template
				std::map<long, std::string> response_parsed = json::parse(response)["FINGER_TABLE"];
				sock->socket_shutdown(2);
				sock->_close();

				// extend fingerCache
				// add the current item 
				finger_cache[item.first] = item.second;
				for (auto& item_suc : response_parsed) {
					finger_cache[item_suc.first] = item_suc.second;
				}
			}
			catch (...){
					cout << "Error while connecting to " << ip << ":" << port << "->" << item.second << endl;
			}
		}
	}

	// append node in notify cache
	for (auto& item : this->notifyCacheTable) {
		finger_cache[item.first] = item.second;
	}
	
	cout << "Finger result :" << this->ndInfos.guid << endl << "Cache : \n";
	// update finger table with the finger cache
	this->fingerTable = std::map<long, std::string>();
	for (int i = 0; i < guids.size(); i++)
	{
		for (auto& item : finger_cache)
		{
			if (item.first >= guids[i] && item.first <= guids[i + 1] && this->fingerTable.find(item.first) == this->fingerTable.end())
			{
				this->fingerTable[item.first] = item.second;
#ifdef DEBUG
				cout <<  item.first << " " << item.second << endl;
#endif
				break;
			}
		}
	}
	
}

void schedule_fingers(PeriodicScheduler& scheduler, CLASS_TEMPLATE& n)
{
	scheduler.addTask("fix_finger", boost::bind(&CLASS_TEMPLATE::FixFingers, &n), 20);
	scheduler.run();
}


template<class usock>
void CLASS_TEMPLATE::StartScheduleJobs(PeriodicScheduler& scheduler) 
{
	if (this->ndInfos.guid == -1) {
		cout << 'GUID not set' << endl;
		throw std::exception();
	}
	this->workers.push_back(std::thread(schedule_fingers, std::ref(scheduler), std::ref(*this)));
	
}

