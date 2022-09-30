#include <cstdio>

#include "LoadBalancer.h"
#include "BootNode.h"
#include "Node.h"
#include <thread>
#include <future>
#include <unistd.h>
#include <string>



void schedule_fingers(PeriodicScheduler& scheduler, Node  &n)
{
	scheduler.addTask("fix_finger", boost::bind(&Node::FixFingers, &n), 15);
	scheduler.run();
}

void schedule_notify(PeriodicScheduler& scheduler, BootNode& n) 
{
	scheduler.addTask("notify", boost::bind(&BootNode::Notify, &n), 30);
	scheduler.run();
}




int main()
{

	PeriodicScheduler scheduler;

	
	srand(time(NULL));
	// instanciate load balancer
	std::vector<std::thread> workers = std::vector<std::thread>();
	LoadBalancer_INFO lb_info;
	lb_info.ip = "127.0.0.1";
	lb_info.port = std::to_string(rand() % 15000 + 4000);
	lb_info.guid = 0;
	LoadBalancer lb = LoadBalancer(lb_info);
	workers.push_back(std::thread(&LoadBalancer::start, &lb));

	// instanciate boot node
	BootNode_INFO bn_info;
	bn_info.ip = "127.0.0.1";
	bn_info.port = std::to_string(rand()%15000 + 4000);
	printf("Creating BootNode ...");
	BootNode nd = BootNode(bn_info);
	workers.push_back(std::thread(&BootNode::start, &nd));
	workers.push_back(std::thread(schedule_notify, std::ref(scheduler), std::ref(nd)));
	nd.Join(lb_info);
	
	// instanciate client node
	
	for (int i = 0; i < 5; i++)
	{
		printf("Creating Node ...");
		Node_INFO nd_info;
		nd_info.ip = "127.0.0.1";
		nd_info.port = std::to_string(rand() % 15000 + 4000);
		printf("Creating Node ...");
		Node * nd = new Node(nd_info);
		workers.push_back(std::thread(&Node::start, std::ref(*nd)));
		nd->Join(lb_info);
		usleep(1000000);
		workers.push_back(std::thread(schedule_fingers, std::ref(scheduler), std::ref(*nd)));
	}

	for (auto& worker : workers)
	{
		worker.join();
	}
}