#pragma once
#include "stdafx.h"
#include <fstream>
#include <boost/serialization/access.hpp>

struct ConnectionInformation {
		friend class boost::serialization::access;

		const static std::string UNUSED_INFO;

		ConnectionInformation();
		ConnectionInformation(const std::string& filePath);
		std::string dsn;
		std::string driver;
		std::string server;
		std::string uid;
		std::string pwd;
		std::string database;

		bool loadFromFile(const std::string& filePath);

		bool saveToFile(const std::string& filePath);

		template<typename Archive>
		void serialize(Archive& ar, const unsigned int version) {
				ar & dsn;
				ar & driver;
				ar & server;
				ar & uid;
				ar & pwd;
				ar & database;
		}
};

class DBManager
{
public:
		static void InitOTL() {
				otl_connect::otl_initialize(1);
		}

		DBManager();

		bool connect(const ConnectionInformation& connectionInfo);

		bool connect(const std::string& connectStr);

		otl_connect* getConnection()
		{
				return dbConnection;
		}

		~DBManager();

protected:
		otl_connect* dbConnection;
};
