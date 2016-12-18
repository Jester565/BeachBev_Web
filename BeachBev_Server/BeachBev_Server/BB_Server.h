#pragma once
#include <WSS_Server.h>

class DBManager;
class CommandHandler;

class BB_Server : public WSS_Server
{
public:
		BB_Server(DBManager* dbManager);

		virtual Client* createClient(boost::shared_ptr<TCPConnection> tcpConnection, IDType id) override;

		DBManager* getDBManager() {
				return dbManager;
		}

		void run(uint16_t port) override;

		~BB_Server();

private:
		
		DBManager* dbManager;
		CommandHandler* cmdHandler;
};

