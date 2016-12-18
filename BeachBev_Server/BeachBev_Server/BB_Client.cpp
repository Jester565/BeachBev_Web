#include "BB_Client.h"


BB_Client::BB_Client(boost::shared_ptr<TCPConnection> tcpConnection, Server * server, IDType id)
		:Client(tcpConnection, server, id)
{
}

BB_Client::~BB_Client()
{
}
