#pragma once
#include "stdafx.h"
#include "DBManager.h"
#include <WSS_Server.h>

class CommandHandler;
class EmployeeManager;

class BB_Server : public WSS_Server
{
public:
	BB_Server();

	virtual void createManagers() override;

	virtual ClientPtr createClient(boost::shared_ptr<TCPConnection> tcpConnection, IDType id) override;

	void setDBConnectionInformation(ConnectionInformation& conInformation) {
		dbConInfo = conInformation;
	}

	EmployeeManager* getEmpManager() {
		return employeeManager;
	}

	void run(uint16_t port) override;

	~BB_Server();

private:
	CommandHandler* cmdHandler;
	EmployeeManager* employeeManager;
	ConnectionInformation dbConInfo;
};
