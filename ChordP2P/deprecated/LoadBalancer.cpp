#include "LoadBalancer.h"


LoadBalancer& LoadBalancer::operator=(const LoadBalancer& other)
{
    // TODO: insérer une instruction return ici
}

void LoadBalancer::Join(Socket* client)
{
    string buffer;
    receiveData(client, buffer);
	cout << "LoadBalancer receive the data: " << buffer << endl;
	// parse json
	json client_inf = json::parse(buffer);

	// close and reconnect
    client->socket_shutdown(2);
    client->close();
	Socket * clientResponse = new Socket(AF_INET, SOCK_STREAM, 0);
    clientResponse->connect(client_inf["ip"], client_inf["port"]);

	
    if (client_inf["type"] == _Client) {
        this->registerNode(client_inf, clientResponse);
    }
    if(client_inf["type"] == _BootNode){
        this->registerBootNode(client_inf, clientResponse);
    }
    clientResponse->close();
    delete clientResponse;
	
}

void LoadBalancer::registerBootNode(json info, Socket* client)
{
    long guid = rand();
    BootNode_INFO tr_info = {
         guid,
        info["ip"],
        info["port"]
    };
    this->bn_networks.push_back(tr_info);

    json data = {
        {"id", PacketID::LB_INIT},
        { "guid", guid }
    };
    client->socket_write(data.dump());
}

void LoadBalancer::registerNode(json info, Socket * client)
{
 
    int masterIndex = rand() % bn_networks.size();
    BootNode_INFO master = this->bn_networks.at(masterIndex);
    json data = {
        {"id", PacketID::LB_INIT},
        {"ip", master.ip  },
        {"port", master.port}
    };
    client->socket_write(data.dump());
}

LoadBalancer::LoadBalancer(LoadBalancer_INFO lb_info) 
{
    this->lb_info = lb_info;
	this->socket = Socket(AF_INET, SOCK_STREAM, 0);	
    this->bn_networks = vector<BootNode_INFO>(0);
}

void LoadBalancer::start(void)
{
    this-> socket.bind(this->lb_info.ip, this->lb_info.port);
    this->socket.listen(10);
    while (true) {
		cout << "Load Balancer is waiting for new connection" << endl;
        vector<Socket> reads(1);
        reads[0] = this->socket;
        int seconds = 10; //Wait 10 seconds for incoming Connections
        if (Socket::select(&reads, NULL, NULL, seconds) < 1) { //Socket::select waits until masterSocket reveives some input (for example a new connection)
            //No new Connection
            continue;
        }
        else {
            //Something happens, let's accept the connection
            Socket* client = this->socket.accept();
			cout << "Load Balancer accept a new connection" << endl;
            this->Join(client);

        }
    }
}

LoadBalancer::~LoadBalancer(void) {
	
}

