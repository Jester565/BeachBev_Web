#include "BB_Server.h"
#include "BB_Client.h"
#include "CommandHandler.h"

BB_Server::BB_Server(DBManager* dbManager)
		:WSS_Server(boost::asio::ip::tcp::v4()), dbManager(dbManager)
{
		cmdHandler = new CommandHandler(this);
}

Client * BB_Server::createClient(boost::shared_ptr<TCPConnection> tcpConnection, IDType id)
{
		return new BB_Client(tcpConnection, this, id);
}

void BB_Server::run(uint16_t port)
{
		WSS_Server::run(port);
		cmdHandler->run();
}

BB_Server::~BB_Server()
{
		delete dbManager;
		dbManager = nullptr;
		delete cmdHandler;
		cmdHandler = nullptr;
}
