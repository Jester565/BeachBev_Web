#include "DBManager.h"
#include "BB_Server.h"
#include <iostream>

static const int SERVER_PORT = 8443;
#ifdef _WIN32
static const std::string CERT_PATH = "C:\\Users\\ajcra\\Desktop\\aws\\SSL\\local.crt";
static const std::string PEM_PATH = "C:\\Users\\ajcra\\Desktop\\aws\\SSL\\local.key";
#else
static const std::string CERT_PATH = "/etc/letsencrypt/live/beachbevs.com/fullchain.pem";
static const std::string PEM_PATH = "/etc/letsencrypt/live/beachbevs.com/privkey.pem";
#endif
static const std::string& CONNECT_INFORMATION_PATH = "./mysql.coni";

int main()
{
		DBManager* dbManager = new DBManager();
		
		if (dbManager->connect(ConnectionInformation(CONNECT_INFORMATION_PATH)))
		{
				BB_Server server(dbManager);
				server.setCertPath(CERT_PATH);
				server.setPemPath(PEM_PATH);
				server.createManagers();
				server.run(SERVER_PORT);
		}
		else
		{
				std::cerr << "Connection failed" << std::endl;
		}
		system("pause");
}
