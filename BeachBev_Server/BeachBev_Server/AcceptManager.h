#pragma once
#include "stdafx.h"
#include <PKeyOwner.h>
#include <google/protobuf/repeated_field.h>
#include "BB_Client.h"

class BB_Server;
class EmailManager;
class MasterManager;

class AcceptManager : public PKeyOwner
{
public:
	static const int INVALID_ASTATE = -2;
	static const int UNVERIFIED_ASTATE = -1;
	static const int UNACCEPTED_ASTATE = 0;
	static const int ACCEPTED_ASTATE = 1;
	AcceptManager(BB_Server* bbServer, MasterManager* masterManager, EmailManager* emailManager);

	void handleE0(boost::shared_ptr<IPacket> iPack);
	bool getEIDsWithAState(int aState, DBManager* dbManager, google::protobuf::RepeatedField<google::protobuf::uint32>* eIDs);

	void handleE2(boost::shared_ptr<IPacket> iPack);
	bool setAState(IDType eID, int aState, DBManager* dbManager);

	void handleE4(boost::shared_ptr<IPacket> iPack);
	int getAState(IDType eID, DBManager* dbManager);

	~AcceptManager();

private:
	BB_Server* bbServer;
	MasterManager* masterManager;
	EmailManager* emailManager;
};

