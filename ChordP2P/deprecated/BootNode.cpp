#include "BootNode.h"

BootNode::BootNode(BootNode_INFO bn_info)
{
    this->bn_info = bn_info;
	this->superGUIDTable = map<long, string>();
	this->superFileTable = map <long, vector<long>>();
    this->clientSocket = Socket(AF_INET, SOCK_STREAM, 0);
    this->srvSocket = Socket(AF_INET, SOCK_STREAM, 0);
}

BootNode::~BootNode(void)
{
}
BootNode::BootNode(const BootNode& other)
{
}
BootNode& BootNode::operator=(BootNode&& other)
{
    // TODO: insérer une instruction return ici
}
int BootNode::Register(Socket* client, json info)
{
    long assigned_guid = -1;
    int counter = 0;
    map<long, std::string> sucessors = map<long, std::string>();
    for (long i = this->RING_SIZE + this->RING_START; i > this->RING_START; i--) {
        if (this->superGUIDTable[i].empty()) {
            assigned_guid = i;
            json client_info = {
                {"ip", info["ip"]},
                {"port", info["port"]}
            };
            this->superGUIDTable[i] = client_info.dump();
            break;
        }
    }
    
    for (long i = assigned_guid + 1; (i <= this->RING_START + this->RING_SIZE) && counter < 5; i++) {
        if (!this->superGUIDTable.at(i).empty()) {
            sucessors[i] = this->superGUIDTable.at(i);
            counter++;
        }
    }
    auto data = RequestBuilder::prepare_client_reponse(sucessors, assigned_guid);
    client->socket_write(data);
    client->close();

    json message = {
        {"id", PacketID::NEW_CLIENT},
        {"ip", client->address},
        {"port", client->port},
        {"guid", assigned_guid}
    };
    this->messagePool.push(message);

}

int BootNode::Unregister(Socket* client, json info)
{
    return 0;
    
}

int BootNode::RescueNode(Socket* client, json info)
{
    return 0;
}

void BootNode::Notify()
{
	// Fixme: forward aux autres BootNode
    std::string msg = RequestBuilder::notify(this->messagePool);
    for (auto& item : this->superGUIDTable)
    {
		json info = json::parse(item.second);
        this->clientSocket = Socket(AF_INET, SOCK_STREAM, 0);
        this->clientSocket.connect(info["ip"], info["port"]);
		this->clientSocket.socket_write(msg);
		this->clientSocket.socket_shutdown(2);
        this->clientSocket.close();
    }
	
}

int BootNode::TransferBlockQuery()
{
    return 0;
}

int BootNode::UpdateSuperTable()
{
    return 0;
}
int BootNode::Join(LoadBalancer_INFO lb_info)
{
	Client_INFO info = { this->bn_info.ip, this->bn_info.port, _BootNode };
	json data = {
		{"ip", info.ip},
		{"port", info.port },
		{"type", info.type }
	};
    Socket clientSocket;
	this->clientSocket.connect(lb_info.ip, lb_info.port);
	this->clientSocket.socket_write(data.dump());
    this->clientSocket.socket_shutdown(2);
    this->clientSocket.close();
    return 0;
}
void BootNode::start(void)
{
    this->srvSocket.bind(this->bn_info.ip, this->bn_info.port);
    this->srvSocket.listen(10);
    while (true) {
        vector<Socket> reads(1);
        reads[0] = this->srvSocket;
        int seconds = 10; //Wait 10 seconds for incoming Connections
        if (Socket::select(&reads, NULL, NULL, seconds) < 1) { //Socket::select waits until masterSocket reveives some input (for example a new connection)
            //No new Connection
            continue;
        }
        else {
            //Something happens, let's accept the connection
            Socket* client = this->srvSocket.accept();
            HandleIncommingRequest(client);
        }
    }
}

void BootNode::HandleIncommingRequest(Socket * client)
{
    string buffer;
    receiveData(client, buffer);
    json data = json::parse(buffer);
    PacketID id = data["id"];
    switch (id)
    {
    case PacketID::LB_INIT:
        this->guid = data["guid"];
        cout << "BootNode succefully registered " << guid << endl;
        break;

    case PacketID::CLIENT_INIT:
        this->Register(client, data);
        break;


    default:
        break;
    }
}
