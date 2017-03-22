#include "DBManager.h"
#include "BB_Server.h"
#include <aws/core/Aws.h>
#include <aws/core/utils/logging/AWSLogging.h>
#include <aws/core/utils/logging/ConsoleLogSystem.h>
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

bool InitAws()
{
	Aws::SDKOptions options;
	Aws::Utils::Logging::InitializeAWSLogging(Aws::MakeShared<Aws::Utils::Logging::ConsoleLogSystem>(AWS_ALLOC_TAG, Aws::Utils::Logging::LogLevel::Trace));
	Aws::InitAPI(options);
}

int main()
{
	InitAws();
	BB_Server server;
	server.setCertPath(CERT_PATH);
	server.setPemPath(PEM_PATH);
	ConnectionInformation conInfo(CONNECT_INFORMATION_PATH);
	server.setDBConnectionInformation(conInfo);
	server.createManagers();
	server.run(SERVER_PORT);
	system("pause");
}
