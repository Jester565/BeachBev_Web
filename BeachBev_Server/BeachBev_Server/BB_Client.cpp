#include "BB_Client.h"

BB_Client::BB_Client(boost::shared_ptr<TCPConnection> tcpConnection, DBManager* dbManager, Server * server, IDType id)
	:Client(tcpConnection, server, id), dbManager(dbManager)
{
}

BB_Client::~BB_Client()
{
}