#include "Node.h"

void Node::Stabilize()
{
}

void Node::FixFingers()
{
    int next = this->nd_info.guid;
	// calcule the perfect finger table
    int finger_table_size = log2(128);
    vector<long> guids = vector<long>(log2(finger_table_size) + 1);
    std::map<long, std::string> fingerCache = std::map<long, std::string>();
	
	for (int i = 0; i < guids.size(); i++)
	{
		guids[i] = next + pow(2, i);
	}
	
    for (int i = 0; i < log2(128); i++) // FIX 128
    {
        for (auto& item : this->fingerTable)
        {
			// query the good node
			if (!(item.first >= guids[i] && item.first <= guids[i + 1]))
                continue;
			
            json sucessor_info = json::parse(item.second);
            string ip = sucessor_info["ip"];
            string port = sucessor_info["port"];
            this->clientSocket = Socket(AF_INET, SOCK_STREAM, 0);
            this->clientSocket.connect(ip, port);
            this->clientSocket.socket_write(RequestBuilder::fix_finger());
            string response;
            receiveData(&this->clientSocket, response);
			// ici faire un template
			std::map<long, std::string> response_parsed = json::parse(response)["finger_table"];
			// extend fingerCache
            fingerCache[item.first] = item.second;
            for (auto& item_suc : response_parsed) {
                fingerCache[item_suc.first] = item_suc.second;
            }
        }
    }

    cout << "Finger result :" << this->nd_info.guid << endl << "Cache : \n";
    for (int i = 0; i < guids.size(); i++)
    {
        this->fingerTable = std::map<long, std::string>();
		for (auto& item : fingerCache)
		{
			if (item.first >= guids[i] && item.first <= guids[i + 1])
			{
				this->fingerTable[item.first] = item.second;
				cout  << item.first << " " << item.second << endl;
                break;
			}
		}
    }
}

void Node::Register(json info)
{
    BootNode_INFO bn_info = { -1, info["ip"], info["port"] };
    this->Register(bn_info);
}

void Node::Join(LoadBalancer_INFO lb_info)
{
	Client_INFO info = { this->nd_info.ip, this->nd_info.port, _Client };
	json data = {
		{"ip", info.ip},
		{"port", info.port },
		{"type", info.type }
	};
	this->clientSocket.connect(lb_info.ip, lb_info.port);
	this->clientSocket.socket_write(data.dump());
    this->clientSocket.socket_shutdown(2);
    this->clientSocket.close();
}

void Node::Register(BootNode_INFO info)
{
	this->masterBootNode = info;
    this->clientSocket = Socket(AF_INET, SOCK_STREAM, 0);
    this->clientSocket.connect(info.ip, info.port);
    std::string data = RequestBuilder::register_client(this->nd_info);
    this->clientSocket.socket_write(data);
    std::string response;
    receiveData(&this->clientSocket, response);
	json response_json = json::parse(response);
	this->nd_info.guid = response_json["guid"];
    map <int, std::string> sucessors = response_json["sucessors"];
	
    for (auto &item : sucessors) {
		cout << item.second << endl;
        this->fingerTable[item.first] = std::string(item.second);
    }
	
}

void Node::leaves()
{
}

void Node::FindSucessor()
{
}

void Node::SendFinger(Socket *  client)
{
	
	client->socket_write(RequestBuilder::send_finger(this->fingerTable));
    client->socket_shutdown(2);
    client->close();
}

Node::Node(Node_INFO nd_info)
{
	this->nd_info = nd_info;
	this->srvSocket = Socket(AF_INET, SOCK_STREAM, 0);
    this->clientSocket = Socket(AF_INET, SOCK_STREAM, 0);

}

Node::~Node(void)
{
}

Node::Node(const Node& other)
{
}

Node& Node::operator=(const Node& other)
{
	// TODO: insérer une instruction return ici
}

void Node::Notify(json info)
{
    cout << "Node Notified :" << nd_info.guid <<" " << info << endl;
	
}

void Node::start(void)
{
    this->srvSocket.bind(this->nd_info.ip, this->nd_info.port);
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
            std::thread(&Node::HandleIncommingRequest, this, client).detach();
        }
    }
}


void Node::HandleIncommingRequest(Socket* client)
{
    string buffer;
    receiveData(client, buffer);
    json data = json::parse(buffer);
    PacketID id = data["id"];
    switch (id)
    {
    case PacketID::LB_INIT:
        this->Register(data);
        break;

	case PacketID::FIX_FINGER:
		// ici faire le traitement, rajouter un ID_OPTIONNAL, double recursion
		// et retourner valeurs
        this->SendFinger(client);    
        break;

    case PacketID::NOTIFY:
        this->Notify(data);
        break;

    default:
        break;
    }
}
