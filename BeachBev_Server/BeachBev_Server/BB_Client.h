#pragma once
#include "stdafx.h"
#include <Client.h>

class DBManager;

class BB_Client : public Client
{
public:
		BB_Client(boost::shared_ptr <TCPConnection> tcpConnection, DBManager* dbManager, Server* server, IDType id);

		DBManager* getDBManager() {
				return dbManager;
		}

		IDType getEmpID() {
				return empID;
		}

		void setEmpID(IDType empID) {
				this->empID = empID;
		}

		~BB_Client();

private:
		DBManager* dbManager;
		IDType empID;
};
