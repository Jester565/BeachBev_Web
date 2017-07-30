#include "BB_Client.h"
#include "BB_Server.h"
#include "EmployeeManager.h"

BB_Client::BB_Client(boost::shared_ptr<TCPConnection> tcpConnection, DBManager* dbManager, Server * server, IDType id)
	:Client(tcpConnection, server, id), dbManager(dbManager)
{
	BB_Server* bbServer = (BB_Server*)server;
	bbServer->getEmpManager()->follow(shared_from_this());
}

BB_Client::~BB_Client()
{
}