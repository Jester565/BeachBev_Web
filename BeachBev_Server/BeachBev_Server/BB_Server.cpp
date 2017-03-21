#include "BB_Server.h"
#include "BB_Client.h"
#include "CommandHandler.h"
#include "BB_ServicePool.h"
#include "EmployeeManager.h"
#include <PacketManager.h>
#include <ClientManager.h>

BB_Server::BB_Server()
	:WSS_Server(boost::asio::ip::tcp::v4())
{
	cmdHandler = new CommandHandler(this);
}

void BB_Server::createManagers()
{
	servicePool = new BB_ServicePool(certPath, pemPath, dbConInfo);
	pm = new PacketManager(this);
	cm = new ClientManager(this);
}

Client * BB_Server::createClient(boost::shared_ptr<TCPConnection> tcpConnection, IDType id)
{
	return new BB_Client(tcpConnection, ((BB_ServicePool*)servicePool)->getNextDBManager(), this, id);
}

void BB_Server::run(uint16_t port)
{
	employeeManager = new EmployeeManager(this);
	WSS_Server::run(port);
	cmdHandler->run();
}

BB_Server::~BB_Server()
{
	delete cmdHandler;
	cmdHandler = nullptr;
}