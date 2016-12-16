#pragma once
#include "stdafx.h"

class DBManager
{
public:
		DBManager();

		bool connect(const std::string& serverName, const std::string& pwd, const std::string& uID, const std::string& dbName, const std::string& driver);

		bool connect(const std::string& driver);

		~DBManager();

protected:
		otl_connect* dbConnection;
};
