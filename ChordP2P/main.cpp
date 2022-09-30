#include "Chord.hpp"
#include "Node.h"
#include"common.hpp"



using usock = Socket;
using uchord = Chord<usock>;
using unode = Node<usock>;
using uchord_ptr = std::shared_ptr <uchord>;
using unode_ptr = std::shared_ptr <unode>;
using vec_pchord = std::vector<uchord_ptr>;
using vec_unode = std::vector<unode_ptr>;


TCP_ADDR helper_create_random_srv_config() 
{
	std::string port = std::to_string(random() % 30000 + 4000);
	auto ip = "127.0.0.1";
	TCP_ADDR tcp_infos = { ip , port };
	usleep(5000);
	return tcp_infos;
}


uchord_ptr helper_create_chord(CHORD_INFO &cinfos)
{
	cinfos.tcpinfos = helper_create_random_srv_config();
	uchord_ptr p(new uchord(cinfos));
	return p;
}

unode_ptr helper_create_node(NODE_INFO& ninfos)
{
	ninfos.tcpinfos = helper_create_random_srv_config();
	unode_ptr p(new unode(ninfos));
	return p;
}


void simulate_env(int nodes_size, int chords_size)
{
	int  CHORD_CLUSTER_SIZE = chords_size;
	int NODES = nodes_size;
	PeriodicScheduler periodic_sheduler;
	auto chords = vec_pchord();
	auto cinfos = std::vector<CHORD_INFO>();
	// instanciate chords nodes
	for (int i = 0; i < CHORD_CLUSTER_SIZE; i++)
	{
		CHORD_INFO cinfo = CHORD_INFO{ i, UrangeChord(i * 128, (i + 1) * 128) };
		uchord_ptr ch = helper_create_chord(cinfo);
		ch->StartScheduleJobs(periodic_sheduler);
		cinfos.push_back(std::move(cinfo));
		chords.push_back(ch);
	}
	//register chors to each other
	for (int i = 0; i < 2; i++)
	{
		chords[i]->RegisterChords(static_cast<std::vector<CHORD_INFO>>(cinfos));
	}
	auto nodes = vec_unode();
	auto ninfos = std::vector<NODE_INFO>();
	// instanciate node
	for (int i = 0; i < (int)NODES/2; i++)
	{
		NODE_INFO ninfo = NODE_INFO{ -1};
		unode_ptr nd = helper_create_node(ninfo);
		// Register a node to a random chord
		nd->Register(cinfos[rand() % CHORD_CLUSTER_SIZE], periodic_sheduler);
		ninfos.push_back(std::move(ninfo));
		nodes.push_back(nd);
	}
	// infinite wait
	usleep(10000000);

	// recreate new node after a short break
	for (int i = 0; i < (int)NODES/2; i++)
	{
		NODE_INFO ninfo = NODE_INFO{ -1 };
		unode_ptr nd = helper_create_node(ninfo);
		// Register a node to a random chord
		nd->Register(cinfos[rand() % CHORD_CLUSTER_SIZE], periodic_sheduler);
		ninfos.push_back(std::move(ninfo));
		nodes.push_back(nd);
	}

	// infinite wait
	while (true)
	{
		usleep(10000000);
	}
	
}

int main(int argc, char* argv[])
{
	int nodes = stoi(argv[1]);
	int chords = stoi(argv[2]);
	srand(time(NULL));
	simulate_env(nodes, chords);
}
