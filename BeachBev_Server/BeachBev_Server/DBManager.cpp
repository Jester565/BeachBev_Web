#include "DBManager.h"



DBManager::DBManager()
{
}

bool DBManager::connect(const std::string & serverName, const std::string & pwd, const std::string & uID, const std::string & dbName, const std::string & driver)
{
		otl_connect::otl_initialize();
		dbConnection = new otl_connect();
		std::string connectStr = "Driver={";
		connectStr += driver;
		connectStr += "}; Server=";
		connectStr += serverName;
		connectStr += "; UID=";
		connectStr += uID;
		connectStr += "; PWD=";
		connectStr += pwd;
		connectStr += "; Database=";
		connectStr += dbName;
		connectStr += ";";
		connectStr += '\0';
		try
		{
				dbConnection->rlogon(connectStr.c_str());
		}
		catch (otl_exception& ex)
		{
				std::cerr << ex.msg << std::endl;
				std::cerr << ex.code << std::endl;
				std::cerr << ex.var_info << std::endl;
				return false;
		}
		return true;
}

bool DBManager::connect(const std::string & driver)
{
		otl_connect::otl_initialize();
		dbConnection = new otl_connect();
		std::string connectStr = "DSN=my-connector";
		try
		{
				dbConnection->rlogon(connectStr.c_str());
		}
		catch (otl_exception& ex)
		{
				std::cerr << ex.msg << std::endl;
				std::cerr << ex.code << std::endl;
				std::cerr << ex.var_info << std::endl;
				return false;
		}
		return true;
}


DBManager::~DBManager()
{
}
