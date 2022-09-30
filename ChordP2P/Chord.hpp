#pragma once

#include "json.hpp"
#include "Periodic.h"
#include "TcpCommunicator.hpp"
#include "common.hpp"

using namespace nlohmann::json_abi_v3_11_2;

#include "definitions.h"

using usock = Socket;
#define CLASS_TEMPLATE Chord<usock>
template <class usock>
class Chord 
{
using TcpComTemplate = TcpCommunicator<usock, CLASS_TEMPLATE>;
public:
	Chord(CHORD_INFO  ch){
		this->chordInfos = ch;
		this->tcpCom = std::make_shared<TcpComTemplate>(TcpComTemplate(ch.tcpinfos, this));
	}
	Chord() = delete;
	~Chord() = default;
	Chord& operator=(const Chord& other) = default;
	Chord(const Chord& other)  = default;

	std::vector<std::thread> workers;


	template <class T>
	void UnregisterChord(T&& infos);

	void RegisterChords(std::vector<CHORD_INFO> infos);

	void HandleIncommingRequest(usock &);

	void Notify();

	void StartScheduleJobs(PeriodicScheduler& scheduler);
	
protected:
	std::shared_ptr<TcpComTemplate> tcpCom;
	UindexTbChord superGUIDTable;
	std::vector<std::string> messagesPool;
	std::vector<CHORD_INFO> chordsNetwork;
	CHORD_INFO chordInfos;
	PeriodicScheduler* scheduler;



	//void HandleIncommingRequest(usock&&) const ;
	
	template <class T>
	void RegisterNode(usock&, T infos);

	template <class T>
	void UnregisterNode(T && infos);

	template <class T>
	void Rescue(usock&&, T infos);

	template <class T>
	void lookup(usock&&, T infos);

	template <class T>
	void registerNotify(T infos);


	
};

template<class usock>
template<class T>
void CLASS_TEMPLATE::UnregisterChord(T&& infos)
{
	return;
}

template<class usock>
template<class T>
void CLASS_TEMPLATE::RegisterNode(usock& client, T infos)
{

	long assigned_guid = -1;
	int successors_list_size = 0;
	const long chord_top = this->chordInfos.range.second;
	const long chord_bottom = this->chordInfos.range.first;
	map<long, std::string> sucessors = map<long, std::string>();
	for (long i = chord_top; i > chord_bottom; i--) {
		
		// FIXME: Check client not already register
		if (0) {

		}

		if (this->superGUIDTable[i].empty()) {
			assigned_guid = i;
			json client_info = {
				{"IP", client.address}, 
				{"PORT", infos["PORT"]} // we set the srv port
			};
			this->superGUIDTable[i] = client_info.dump();
			break;
		}
	}

	for (long i = assigned_guid + 1; (i <= chord_top) && successors_list_size < 5; i++) {
		if (!this->superGUIDTable.at(i).empty()) {
			sucessors[i] = this->superGUIDTable.at(i);
			successors_list_size++;
		}
	}
	auto data = RequestBuilder::PrepareClientReponse(sucessors, assigned_guid);
	client.socket_write(data);
	client.close();

	json message = {
		{"PACKET_ID", PacketID::CLIENT_JOINED},
		{"IP", client.address},
		{"PORT", infos["PORT"]}, // we set the srv port
		{"GUID", assigned_guid}
	};
	this->messagesPool.push_back(message.dump());

}

template<class usock>
template<class T>
void CLASS_TEMPLATE::UnregisterNode(T&& chord)
{
}
template<class usock>
void CLASS_TEMPLATE::RegisterChords(std::vector<CHORD_INFO> chords)
{
	this->chordsNetwork = chords;
	
}
template<class usock>
template<class T>
void CLASS_TEMPLATE::Rescue(usock&&, T infos)
{
}

template<class usock>
template<class T>
void CLASS_TEMPLATE::lookup(usock&&, T infos)
{
}

template<class usock>
template<class T>
void CLASS_TEMPLATE::registerNotify(T infos)
{
	// forward to each nodes 
	// critical section
	std::mutex mtx;
	mtx.lock();
	auto notify = [](std::string ip, std::string port, auto* instance, auto&& request) {
		instance->tcpCom->clientSocket = std::make_shared<usock>(usock(AF_INET, SOCK_STREAM, 0));
		auto  sock = instance->tcpCom->clientSocket;
		sock->connect(ip, port);
		sock->socket_write(request);
		sock->socket_shutdown(2);
		sock->close();
	};

	const long chord_top = this->chordInfos.range.second;
	const long chord_bottom = this->chordInfos.range.first;
	for (long i = chord_top; i > chord_bottom; i--) {

		if (!this->superGUIDTable[i].empty()) {
			auto node = this->superGUIDTable[i];
			std::string ip = json::parse(node)["IP"];
			std::string port = json::parse(node)["PORT"];
			notify(ip, port, this, infos);
		}
	}
	mtx.unlock();

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
	case PacketID::CLIENT_INIT:
		this->RegisterNode<json>(client, data);
		break;

	case PacketID::NOTIFY:
		this->registerNotify(buffer);

	default:
		break;
	}
	
}

void schedule_notify(PeriodicScheduler& scheduler, CLASS_TEMPLATE& n)
{
	scheduler.addTask("notify everyone", boost::bind(&CLASS_TEMPLATE::Notify, &n), 10);
	scheduler.run();
}



template<class usock>
void CLASS_TEMPLATE::StartScheduleJobs(PeriodicScheduler& scheduler)
{
	this->workers.push_back(std::thread(schedule_notify, std::ref(scheduler), std::ref(*this)));

}
template<class usock>
void CLASS_TEMPLATE::Notify() 
{
	// Forward to other chords
#ifdef  DEBUG
	cout << "Chord Notify launch. Spreading new clients over the network." << endl;
#endif //  DEBUG
	std::string request = RequestBuilder::PrepareNotify(this->messagesPool);
	if (this->messagesPool.size() < 1) {
		return;
	}
	auto notify = [](std::string ip, std::string port, auto* instance, auto& request) {
		instance->tcpCom->clientSocket = std::make_shared<usock>(usock(AF_INET, SOCK_STREAM, 0));
		auto  sock = instance->tcpCom->clientSocket;
		sock->connect(ip, port);
		sock->socket_write(request);
		sock->socket_shutdown(2);
		sock->close();
	};

	for (auto& chord : this->chordsNetwork) {
		notify(chord.tcpinfos.srv_ip, chord.tcpinfos.srv_port, this, request);
	}
	// clear it
	this->messagesPool = std::vector<std::string>();
}
