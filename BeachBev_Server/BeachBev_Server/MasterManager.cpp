#include "MasterManager.h"
#include "DBManager.h"

MasterManager::MasterManager(BB_Server * bbServer)
	:bbServer(bbServer)
{
}

bool MasterManager::isMaster(IDType eID, DBManager * dbManager)
{
	std::string query = "SELECT eID FROM Masters WHERE eID=:f1<int>";
	try {
		otl_stream otlStream(OTL_BUFFER_SIZE, query.c_str(), *dbManager->getConnection());
		otlStream << (int)eID;
		if (!otlStream.eof()) {
			return true;
		}
	}
	catch (otl_exception ex) {
		std::cerr << "Code: " << ex.code << std::endl << "MSG: " << ex.msg << std::endl;
	}
	return false;
}

MasterManager::~MasterManager()
{
}
