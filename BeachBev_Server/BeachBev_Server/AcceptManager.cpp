#include "AcceptManager.h"
#include "BB_Server.h"
#include "DBManager.h"
#include "MasterManager.h"
#include "EmailManager.h"
#include <ClientManager.h>
#include <WSIPacket.h>
#include <WSOPacket.h>
#include "Packets/BBPacks.pb.h"

AcceptManager::AcceptManager(BB_Server * bbServer, MasterManager * masterManager, EmailManager * emailManager)
	:PKeyOwner(), bbServer(bbServer), masterManager(masterManager), emailManager(emailManager)
{
	addKey(boost::make_shared<PKey>("E0", this, &AcceptManager::handleE0));
	addKey(boost::make_shared<PKey>("E2", this, &AcceptManager::handleE2));
	addKey(boost::make_shared<PKey>("E4", this, &AcceptManager::handleE4));
	addKey(boost::make_shared<PKey>("E6", this, &AcceptManager::handleE6));
}

void AcceptManager::handleE0(boost::shared_ptr<IPacket> iPack)
{
	BB_ClientPtr sender = boost::static_pointer_cast<BB_Client>(iPack->getSender());
	if (sender == nullptr) {
		return;
	}
	DBManager* dbManager = sender->getDBManager();
	ProtobufPackets::PackE1 packE1;
	packE1.set_success(false);
	if (masterManager->isMaster(sender->getEmpID(), dbManager)) {
		if (getEIDsWithAState(UNACCEPTED_ASTATE, dbManager, packE1.mutable_unacceptedeids())) {
			if (getEIDsWithAState(ACCEPTED_ASTATE, dbManager, packE1.mutable_acceptedeids()))
			{
				packE1.set_success(true);
			}
			else
			{
				packE1.set_msg("Aquring eIDs of ACCEPTED_ASTATE failed");
			}
		}
		else
		{
			packE1.set_msg("Aquring eIDs of UNACCEPTED_ASTATE failed");
		}
	}
	else
	{
		packE1.set_msg("Not logged in as master");
	}
	auto oPack = boost::make_shared<WSOPacket>("E1");
	oPack->setSenderID(0);
	oPack->addSendToID(sender->getID());
	oPack->setData(boost::make_shared<std::string>(packE1.SerializeAsString()));
	bbServer->getClientManager()->send(oPack, sender);
}

bool AcceptManager::getEIDsWithAState(int aState, DBManager * dbManager, google::protobuf::RepeatedField<google::protobuf::uint32>* eIDs)
{
	std::string query = "SELECT eID FROM Employees WHERE aState=:f1<int>";
	try {
		otl_stream otlStream(OTL_BUFFER_SIZE, query.c_str(), *dbManager->getConnection());
		otlStream << aState;
		while (!otlStream.eof()) {
			int eID;
			otlStream >> eID;
			eIDs->Add(eID);
		}
		return true;
	}
	catch (otl_exception ex) {
		std::cerr << "Code: " << ex.code << std::endl << "MSG: " << ex.msg << std::endl;
	}
	return false;
}

void AcceptManager::handleE2(boost::shared_ptr<IPacket> iPack)
{
	BB_ClientPtr sender = boost::static_pointer_cast<BB_Client>(iPack->getSender());
	if (sender == nullptr) {
		return;
	}
	DBManager* dbManager = sender->getDBManager();
	ProtobufPackets::PackE3 replyPacket;
	replyPacket.set_success(false);
	if (masterManager->isMaster(sender->getEmpID(), dbManager)) {
		ProtobufPackets::PackE2 packE2;
		packE2.ParseFromString(*iPack->getData());
		if (setAState(packE2.eid(), packE2.astate(), dbManager)) {
			replyPacket.set_success(true);
			replyPacket.set_eid(packE2.eid());
			if (packE2.astate() == ACCEPTED_ASTATE) {
				sendAcceptEmail(packE2.eid(), sender->getDBManager());
			}
		}
		else
		{
			replyPacket.set_msg("Failed to set aState");
		}
	}
	else
	{
		replyPacket.set_msg("Must be a master");
	}
	auto oPack = boost::make_shared<WSOPacket>("E3");
	oPack->setSenderID(0);
	oPack->addSendToID(sender->getID());
	oPack->setData(boost::make_shared<std::string>(replyPacket.SerializeAsString()));
	bbServer->getClientManager()->send(oPack, sender);
}

bool AcceptManager::setAState(IDType eID, int aState, DBManager * dbManager)
{
	std::string query = "UPDATE Employees SET aState=:f1<int> WHERE eID=:f2<int>";
	try {
		otl_stream otlStream(OTL_BUFFER_SIZE, query.c_str(), *dbManager->getConnection());
		otlStream << aState;
		otlStream << (int)eID;
		return true;
	}
	catch (otl_exception ex) {
		std::cerr << "Code: " << ex.code << std::endl << "MSG: " << ex.msg << std::endl;
	}
	return false;
}

void AcceptManager::handleE4(boost::shared_ptr<IPacket> iPack)
{
	BB_ClientPtr sender = boost::static_pointer_cast<BB_Client>(iPack->getSender());
	if (sender == nullptr) {
		return;
	}
	DBManager* dbManager = sender->getDBManager();
	ProtobufPackets::PackE5 replyPacket;
	replyPacket.set_astate(getAState(sender->getEmpID(), dbManager));
	auto oPack = boost::make_shared<WSOPacket>("E5");
	oPack->setSenderID(0);
	oPack->addSendToID(sender->getID());
	oPack->setData(boost::make_shared<std::string>(replyPacket.SerializeAsString()));
	bbServer->getClientManager()->send(oPack, sender);
}

int AcceptManager::getAState(IDType eID, DBManager * dbManager)
{
	std::string query = "SELECT aState FROM Employees WHERE eID=:f1<int>";
	try {
		otl_stream otlStream(OTL_BUFFER_SIZE, query.c_str(), *dbManager->getConnection());
		otlStream << (int)eID;
		if (!otlStream.eof()) {
			int aState;
			otlStream >> aState;
			return aState;
		}
	}
	catch (otl_exception ex) {
		std::cerr << "Code: " << ex.code << std::endl << "MSG: " << ex.msg << std::endl;
	}
	return INVALID_ASTATE;
}

void AcceptManager::handleE6(boost::shared_ptr<IPacket> iPack)
{
	BB_ClientPtr sender = boost::static_pointer_cast<BB_Client>(iPack->getSender());
	ProtobufPackets::PackE6 packE6;
	packE6.ParseFromString(*iPack->getData());
	ProtobufPackets::PackE7 replyPacket;
	replyPacket.set_msg("Failed to set aState");
	if (packE6.accept()) {
		replyPacket.set_success(setAState(sender->getEmpID(), EMPLOYEE_ASTATE, sender->getDBManager()));
	}
	else
	{
		replyPacket.set_success(setAState(sender->getEmpID(), DECLINE_ASTATE, sender->getDBManager()));
	}
	auto oPack = boost::make_shared<WSOPacket>("E7");
	oPack->setSenderID(0);
	oPack->addSendToID(sender->getID());
	oPack->setData(boost::make_shared<std::string>(replyPacket.SerializeAsString()));
	bbServer->getClientManager()->send(oPack, sender);
}

void AcceptManager::sendAcceptEmail(IDType eID, DBManager * dbManager)
{
	std::string email;
	if (emailManager->getVerifiedEmail(eID, email, dbManager)) {
		std::stringstream stringIn;
		{
			std::ifstream fileIn(EmailManager::HTML_DIR + "accept.html");
			stringIn << fileIn.rdbuf();
		}
		if (!emailManager->sendEmail(email, "management@beachbevs.com", "BeachBevs",
			"Accepted!", stringIn.str(), std::bind(&AcceptManager::acceptEmailHandler, this,
				std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), nullptr, true)) {
			std::cerr << "Send acceptance email failed to run" << std::endl;
		}
	}
}

void AcceptManager::acceptEmailHandler(const Aws::SES::SESClient * client, const Aws::SES::Model::SendEmailRequest & request, const Aws::SES::Model::SendEmailOutcome & outcome)
{
	if (!outcome.IsSuccess()) {
		std::cerr << "ERROR SENDING ACCEPTANCE EMAIL: " << AwsErrorToStr(outcome.GetError()) << std::endl;
	}
}

AcceptManager::~AcceptManager()
{
}
