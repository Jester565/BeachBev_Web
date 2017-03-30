#pragma once
#include "stdafx.h"
#include "BB_Client.h"

class BB_Server;

class MasterManager
{
public:
	MasterManager(BB_Server* bbServer);

	bool isMaster(IDType eID, DBManager* dbManager);

	~MasterManager();

private:
	BB_Server* bbServer;
};

