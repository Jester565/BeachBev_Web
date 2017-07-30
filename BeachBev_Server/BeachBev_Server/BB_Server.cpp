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
	cm = new ClientManager(this);
}

ClientPtr BB_Server::createClient(boost::shared_ptr<TCPConnection> tcpConnection, IDType id)
{
	DBManager* dbManager = ((BB_ServicePool*)servicePool)->getNextDBManager();
	BB_ClientPtr bbClient = boost::make_shared<BB_Client>(tcpConnection, dbManager, this, id);
	bbClient->init();
	return boost::static_pointer_cast<Client>(bbClient);	
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
