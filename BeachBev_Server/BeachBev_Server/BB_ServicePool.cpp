#include "BB_ServicePool.h"

BB_ServicePool::BB_ServicePool(const std::string & certFile, const std::string & keyFile, const std::string & dbConfigFile, int numUsedCores)
	:BB_ServicePool(certFile, keyFile, ConnectionInformation(dbConfigFile), numUsedCores)
{
}

BB_ServicePool::BB_ServicePool(const std::string & certFile, const std::string & keyFile, ConnectionInformation dbConfig, int numUsedCores)
	: WSS_ServicePool(certFile, keyFile, numUsedCores)
{
	DBManager::InitOTL();
	for (int i = 0; i < numCores; i++) {
		DBManager* dbManager = new DBManager();
		dbManager->connect(dbConfig);
		dbManagers.push_back(dbManager);
	}
	dbManagerIter = dbManagers.begin();
}

BB_ServicePool::~BB_ServicePool()
{
	for (int i = 0; i < dbManagers.size(); i++) {
		delete dbManagers.at(i);
	}
	dbManagers.clear();
}