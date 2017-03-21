#pragma once
#include <WSS_ServicePool.h>
#include "DBManager.h"
#include <vector>

class DBManager;
class ConnectionInformation;

class BB_ServicePool : public WSS_ServicePool
{
public:
	BB_ServicePool(const std::string& certFile, const std::string& keyFile, const std::string& dbConfigFile, int numUsedCores = 1);

	BB_ServicePool(const std::string& certFile, const std::string& keyFile, ConnectionInformation dbConfig, int numUsedCores = 1);

	DBManager* getNextDBManager() {
		if (dbManagerIter >= dbManagers.end()) {
			dbManagerIter = dbManagers.begin();
		}
		return *(dbManagerIter++);
	}

	~BB_ServicePool();

private:
	std::vector <DBManager*> dbManagers;
	std::vector <DBManager*>::iterator dbManagerIter;
};
