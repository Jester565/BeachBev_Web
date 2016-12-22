#include "DBManager.h"
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

typedef boost::archive::text_oarchive ArchiveOut;
typedef boost::archive::text_iarchive ArchiveIn;

const std::string ConnectionInformation::UNUSED_INFO = "i";

ConnectionInformation::ConnectionInformation()
{
		dsn = UNUSED_INFO;
		driver = UNUSED_INFO;
		server = UNUSED_INFO;
		uid = UNUSED_INFO;
		pwd = UNUSED_INFO;
		database = UNUSED_INFO;
}

ConnectionInformation::ConnectionInformation(const std::string & filePath)
{
		dsn = UNUSED_INFO;
		driver = UNUSED_INFO;
		server = UNUSED_INFO;
		uid = UNUSED_INFO;
		pwd = UNUSED_INFO;
		database = UNUSED_INFO;
		loadFromFile(filePath);
}

bool ConnectionInformation::loadFromFile(const std::string & filePath)
{
		std::ifstream fileIn(filePath);
		if (fileIn.is_open())
		{
				try {
						ArchiveIn arcIn(fileIn);
						arcIn >> *this;  //no need to close?
						return true;
				}
				catch (boost::archive::archive_exception ex)
				{
						std::cerr << ex.code << ": " << ex.what() << std::endl;
				}
		}
		return false;
}

bool ConnectionInformation::saveToFile(const std::string & filePath)
{
		std::ofstream fileOut(filePath);
		if (fileOut.is_open())
		{
				try {
						ArchiveOut arcOut(fileOut);
					 arcOut << *this;  //no need to close?
						return true;
				}
				catch (boost::archive::archive_exception ex) {
						std::cerr << ex.code << ": " << ex.what() << std::endl;
				}
		}
		return false;
}


DBManager::DBManager()
		:dbConnection(nullptr)
{
}

bool DBManager::connect(const ConnectionInformation& connectionInfo)
{
		std::string connectStr;
		if (connectionInfo.dsn != ConnectionInformation::UNUSED_INFO)
		{
				connectStr += "DSN=";
				connectStr += connectionInfo.dsn;
				connectStr += "; ";
		}
		if (connectionInfo.driver != ConnectionInformation::UNUSED_INFO)
		{
				connectStr += "Driver={";
				connectStr += connectionInfo.driver;
				connectStr += "}; ";
		}
		if (connectionInfo.server != ConnectionInformation::UNUSED_INFO)
		{
				connectStr += "Server=";
				connectStr += connectionInfo.server;
				connectStr += "; ";
		}
		if (connectionInfo.uid != ConnectionInformation::UNUSED_INFO)
		{
				connectStr += "UID=";
				connectStr += connectionInfo.uid;
				connectStr += "; ";
		}
		if (connectionInfo.pwd != ConnectionInformation::UNUSED_INFO)
		{
				connectStr += "PWD=";
				connectStr += connectionInfo.pwd;
				connectStr += "; ";
		}
		if (connectionInfo.database != ConnectionInformation::UNUSED_INFO)
		{
				connectStr += "Database=";
				connectStr += connectionInfo.database;
				connectStr += "; ";
		}
		connectStr += '\0';
		std::cout << connectStr << std::endl;
		return connect(connectStr);
}

bool DBManager::connect(const std::string & connectStr)
{
		dbConnection = new otl_connect();
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
		if (dbConnection != nullptr) {
				delete dbConnection;
				dbConnection = nullptr;
		}
}