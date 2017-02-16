#include "stdafx.h"
#include "EmployeeManager.h"
#include "Packets/BBPacks.pb.h"
#include "BB_Server.h"
#include "DBManager.h"
#include "BB_Client.h"
#include "CryptoManager.h"
#include "EmailManager.h"
#include "DebugManager.h"
#include <WSS_TCPConnection.h>
#include <WSOPacket.h>
#include <ClientManager.h>
#include <PKey.h>
#include <time.h>
#include <thread>
#include <csignal>

bool EmployeeManager::CheckInTimeRange(OTL_BIGINT& time, int numHours) {
	OTL_BIGINT now = std::time(NULL);
	return (now - time <= numHours * 60 * 60);
}

EmployeeManager::EmployeeManager(BB_Server* bbServer)
		:PKeyOwner(bbServer->getPacketManager()), bbServer(bbServer)
{
		addKey(new PKey("A0", this, &EmployeeManager::handleA0));
		addKey(new PKey("A2", this, &EmployeeManager::handleA2));
		addKey(new PKey("A3", this, &EmployeeManager::handleA3));
		emailManager = new EmailManager(bbServer, this);
}

void EmployeeManager::handleA0(boost::shared_ptr<IPacket> iPack)
{
	std::raise(SIGINT);
	std::cout << "A0 called" << std::endl;
		ProtobufPackets::PackA0 packA0;
		packA0.ParseFromString(*iPack->getData());
		ProtobufPackets::PackA1 replyPacket;
		BB_Client* sender = (BB_Client*)(bbServer->getClientManager()->getClient(iPack->getSentFromID()));
		if (sender == nullptr) {
				return;
		}
		DBManager* dbManager = sender->getDBManager();
		std::cout << "Name: " << packA0.name() << std::endl;
		IDType eID = nameToEID(packA0.name(), dbManager);
		if (eID == 0) {
			std::cout << "email to eID" << std::endl;
				eID = emailManager->emailToEID(packA0.email(), dbManager);
				std::cout << "MOO" << std::endl;
				if (eID == 0) {
					std::cout << "call 1" << std::endl;
					eID = addEmployeeToDatabase(packA0.name(), dbManager);
					std::cout << "call 2" << std::endl;
						setPwd(eID, packA0.pwd(), dbManager);
						std::cout << "Pwd set" << std::endl;
						std::string urlEncodedPwdToken;
						DeviceID devID = addPwdToken(eID, urlEncodedPwdToken, dbManager);
						std::string urlEncodedEmailToken;
						std::cout << "pwdToken set" << std::endl;
						emailManager->setUnverifiedEmail(eID, packA0.email(), urlEncodedEmailToken, dbManager);
						emailManager->sendVerificationEmail(packA0.email(), urlEncodedEmailToken);
						loginClient(sender, eID);

						replyPacket.set_pwdtoken(urlEncodedPwdToken);
						replyPacket.set_deviceid(devID);
						replyPacket.set_eid(eID);
						replyPacket.set_msg("Account Added");
				}
				else
				{
						replyPacket.set_msg("Email already used");
				}
		}
		else
		{
				replyPacket.set_msg("Name already used");
		}
		std::cout << "Sending..." << std::endl;
		replyPacket.set_eid(0);
		boost::shared_ptr<OPacket> oPack = boost::make_shared<OPacket>("A1");
		oPack->setSenderID(0);
		oPack->addSendToID(sender->getID());
		oPack->setData(boost::make_shared<std::string>(replyPacket.SerializeAsString()));
		bbServer->getClientManager()->send(oPack, sender);
}

void EmployeeManager::handleA2(boost::shared_ptr<IPacket> iPack)
{
		ProtobufPackets::PackA2 packA2;
		packA2.ParseFromString(*iPack->getData());
		ProtobufPackets::PackA1 replyPacket;
		BB_Client* sender = (BB_Client*)(bbServer->getClientManager()->getClient(iPack->getSentFromID()));
		if (sender == nullptr) {
				return;
		}
		DBManager* dbManager = sender->getDBManager();
		BYTE dbTokenHash[TOKEN_SIZE];
		OTL_BIGINT tokenTime;
		if (getPwdToken(packA2.eid(), dbTokenHash, tokenTime, packA2.deviceid(), dbManager)) {
				if (CheckInTimeRange(tokenTime, MAX_TOKEN_HOURS)) {
						std::vector<BYTE> packToken;
						packToken.reserve(TOKEN_SIZE);
						CryptoManager::UrlDecode(packToken, packA2.pwdtoken());
						BYTE packTokenHash[TOKEN_SIZE];
						CryptoManager::GenerateHash(packTokenHash, TOKEN_SIZE, packToken.data(), packToken.size());
						bool match = true;
						for (int i = 0; i < TOKEN_SIZE; i++)		//Iterate through all to prevent time-based attacks
						{
								if (packTokenHash[i] != dbTokenHash[i]) {
										match = false;
								}
						}
						if (match) {
								std::string urlEncodedPwdToken;
								setPwdToken(packA2.eid(), urlEncodedPwdToken, packA2.deviceid(), dbManager);
								replyPacket.set_pwdtoken(urlEncodedPwdToken);
								replyPacket.set_eid(packA2.eid());
								replyPacket.set_deviceid(packA2.deviceid());
								replyPacket.set_msg("Login successful");
								loginClient(sender, packA2.eid());
						}
						else
						{
								replyPacket.set_msg("Tokens did not match");
						}
				}
				else
				{
						replyPacket.set_msg("Token expired");
				}
		}
		else
		{
				replyPacket.set_msg("Could not aquire a token");
		}
		boost::shared_ptr<OPacket> oPack = boost::make_shared<OPacket>("A1");
		oPack->setSenderID(0);
		oPack->setData(boost::make_shared<std::string>(replyPacket.SerializeAsString()));
		bbServer->getClientManager()->send(oPack, sender);
}

void EmployeeManager::handleA3(boost::shared_ptr<IPacket> iPack)
{
		ProtobufPackets::PackA3 packA3;
		packA3.ParseFromString(*iPack->getData());
		ProtobufPackets::PackA1 replyPacket;
		BB_Client* sender = (BB_Client*)(bbServer->getClientManager()->getClient(iPack->getSentFromID()));
		if (sender == nullptr) {
				return;
		}
		DBManager* dbManager = sender->getDBManager();
		IDType eID = nameToEID(packA3.name(), dbManager);
		if (eID != 0) {
				BYTE dbPwdHash[HASH_SIZE];
				BYTE dbPwdSalt[SALT_SIZE];
				if (getPwdData(eID, dbPwdHash, dbPwdSalt, dbManager)) {
						BYTE packPwdHash[HASH_SIZE];
						CryptoManager::GenerateHash(packPwdHash, HASH_SIZE, 
								(BYTE*)packA3.pwd().data(), packA3.pwd().size(),
								dbPwdSalt, SALT_SIZE);
						bool match = true;
						for (int i = 0; i < HASH_SIZE; i++) {
								if (packPwdHash[i] != dbPwdHash[i]) {
										match = false;
								}
						}
						if (match) {
								std::string urlEncodedPwdToken;
								DeviceID devID = packA3.deviceid();
								if (devID != 0) {
										setPwdToken(eID, urlEncodedPwdToken, packA3.deviceid(), dbManager);
								}
								else {
										devID = addPwdToken(eID, urlEncodedPwdToken, dbManager);
								}
								replyPacket.set_pwdtoken(urlEncodedPwdToken);
								replyPacket.set_eid(eID);
								replyPacket.set_deviceid(devID);
								replyPacket.set_msg("Login successful");
								loginClient(sender, eID);
						}
						else
						{
								replyPacket.set_msg("Invalid login");
						}
				}
				else
				{
						replyPacket.set_msg("Could not get pwd data from database");
				}
		}
		else
		{
				replyPacket.set_msg("Invalid login");
		}
		boost::shared_ptr<OPacket> oPack = boost::make_shared<OPacket>("A1");
		oPack->setSenderID(0);
		oPack->setData(boost::make_shared<std::string>(replyPacket.SerializeAsString()));
		bbServer->getClientManager()->send(oPack, sender);
}

BB_Client* EmployeeManager::getEmployee(IDType eID)
{
		auto it = employees.find(eID);
		if (it != employees.end())
		{
				return (BB_Client*)it->second;
		}
		return nullptr;
}

IDType EmployeeManager::addEmployeeToDatabase(const std::string & name, DBManager * dbManager)
{
		IDType eID = getNextEID(dbManager);
		std::string query = "INSERT INTO Employees (eID, name) VALUES (:f1<int>, :f2<char[";
		query += std::to_string(NAME_SIZE);
		query += "]>)";
		try {
				otl_stream otlStream(OTL_BUFFER_SIZE, query.c_str(), *dbManager->getConnection());
				otlStream << (int)eID;
				otlStream << name;
		}
		catch (otl_exception ex)
		{
				std::cerr << "Code: " << ex.code << std::endl << "MSG: " << ex.msg << std::endl;
		}
		return eID;
}

bool EmployeeManager::setPwd(IDType eID, const std::string & pwd, DBManager * dbManager)
{
		clearPwdTokens(eID, dbManager);
		BYTE genSalt[SALT_SIZE];
		CryptoManager::GenerateRandomData(genSalt, SALT_SIZE);
		BYTE genHash[HASH_SIZE];
		CryptoManager::GenerateHash(genHash, HASH_SIZE, (BYTE*)pwd.data(), pwd.size(), genSalt, SALT_SIZE);
		
		std::string query = "UPDATE Employees SET pwdHash=:f1<raw[";
		query += std::to_string(HASH_SIZE);
		query += "]>, pwdSalt=:f2<raw[";
		query += std::to_string(SALT_SIZE);
		query += "]> WHERE eID = :f3<int>";
		try {
				otl_stream otlStream(OTL_BUFFER_SIZE, query.c_str(), *dbManager->getConnection());
				CryptoManager::OutputBytes(otlStream, genHash, HASH_SIZE);
				CryptoManager::OutputBytes(otlStream, genSalt, SALT_SIZE);
				otlStream << (int)eID;
		}
		catch (otl_exception ex)
		{
				std::cerr << "Code: " << ex.code << std::endl << "MSG: " << ex.msg << std::endl;
				return false;
		}
		return true;
}

bool EmployeeManager::setPwdToken(IDType eID, std::string& urlEncodedPwdToken, DeviceID devID, DBManager * dbManager)
{
		BYTE genToken[TOKEN_SIZE];
		CryptoManager::GenerateRandomData(genToken, TOKEN_SIZE);
		BYTE genTokenHash[TOKEN_SIZE];
		CryptoManager::GenerateHash(genTokenHash, TOKEN_SIZE, genToken, TOKEN_SIZE);
		std::string query = "REPLACE INTO PwdTokens VALUES (:f1<int>, :f2<int>, :f3<raw[";
		query += std::to_string(TOKEN_SIZE);
		query += "]>, :f4<bigint>)";
		try {
				otl_stream otlStream(OTL_BUFFER_SIZE, query.c_str(), *dbManager->getConnection());
				otlStream << (int)eID;
				otlStream << (int)devID;
				CryptoManager::OutputBytes(otlStream, genTokenHash, TOKEN_SIZE);
				otlStream << (OTL_BIGINT)(std::time(NULL));
		}
		catch (otl_exception ex)
		{
				std::cerr << "Code: " << ex.code << std::endl << "MSG: " << ex.msg << std::endl;
		}
		CryptoManager::UrlEncode(urlEncodedPwdToken, genToken, TOKEN_SIZE);
		return true;
}

bool EmployeeManager::clearPwdTokens(IDType eID, DBManager * dbManager)
{
		std::string query = "DELETE FROM PwdTokens WHERE eID=:f1<int>";
		try {
				otl_stream otlStream(OTL_BUFFER_SIZE, query.c_str(), *dbManager->getConnection());
				otlStream << (int)eID;
		}
		catch (otl_exception ex)
		{
				std::cerr << "Code: " << ex.code << std::endl << "MSG: " << ex.msg << std::endl;
		}
}

DeviceID EmployeeManager::addPwdToken(IDType eID, std::string & urlEncodedPwdToken, DBManager * dbManager)
{
		DeviceID devID = getNextDeviceID(eID, dbManager);
		setPwdToken(eID, urlEncodedPwdToken, devID, dbManager);
		return devID;
}

DeviceID EmployeeManager::getNextDeviceID(IDType eID, DBManager * dbManager)
{
		DeviceID devID = 0;
		std::string query = "SELECT deviceID FROM PwdTokens ORDER BY deviceID desc limit 1 WHERE eID = :f1<int>";
		try
		{
				otl_stream otlStream(OTL_BUFFER_SIZE, query.c_str(), *dbManager->getConnection());
				otlStream << (int)eID;
				if (!otlStream.eof())
				{
						int devInt = 0;
						otlStream >> devInt;
						devID = devInt;
				}
		}
		catch (otl_exception ex)
		{
				std::cerr << "Code: " << ex.code << std::endl << "MSG: " << ex.msg << std::endl;
		}
		return devID + 1;
}

IDType EmployeeManager::nameToEID(const std::string& name, DBManager* dbManager)
{
		IDType eID = 0;
		std::string query = "SELECT eID FROM Employees WHERE name = :f1<char[";
		query += std::to_string((int)NAME_SIZE);
		query += "]>";
		try
		{
			if (dbManager->getConnection() == nullptr) {
				std::cout << "REE" << std::endl;
			}
			std::cout << "Running otl" << std::endl;
				otl_stream otlStream(OTL_BUFFER_SIZE, query.c_str(), *dbManager->getConnection());
				std::cout << "Running otl2" << std::endl;
				otlStream << name;
				std::cout << "output name" << std::endl;
				if (!otlStream.eof()) {
					std::cout << "Running otl3" << std::endl;
						int eIDInt = 0;
						otlStream >> eIDInt;
						std::cout << "Running otl4" << std::endl;
						eID = eIDInt;
						std::cout << "running otl5" << std::endl;
				}
		}
		catch (otl_exception ex)
		{
				std::cerr << "Code: " << ex.code << std::endl << "MSG: " << ex.msg << std::endl;
		}
		return eID;
}

IDType EmployeeManager::getNextEID(DBManager* dbManager)
{
		IDType eID = 0;
		std::string query = "SELECT * FROM Employees ORDER BY eID desc limit 1";
		try
		{
				otl_stream otlStream(OTL_BUFFER_SIZE, query.c_str(), *dbManager->getConnection());
				dbManager->getConnection()->commit();
				if (!otlStream.eof())
				{
						int eIDInt = 0;
						otlStream >> eIDInt;
						eID = eIDInt;
				}
		}
		catch (otl_exception ex)
		{
				std::cerr << "Code: " << ex.code << std::endl << "MSG: " << ex.msg << std::endl;
		}
		return eID + 1;
}

bool EmployeeManager::getPwdData(IDType eID, BYTE * hash, BYTE * salt, DBManager * dbManager)
{
		std::string query = "SELECT pwdHash, pwdSalt FROM Employees WHERE eID = :f1<int>";
		try {
				otl_stream otlStream(OTL_BUFFER_SIZE, query.c_str(), *dbManager->getConnection());
				otlStream << (int)eID;
				if (!otlStream.eof()) {
						CryptoManager::InputBytes(otlStream, hash, HASH_SIZE);
						CryptoManager::InputBytes(otlStream, salt, SALT_SIZE);
						return true;
				}
		}
		catch (otl_exception ex)
		{
				std::cerr << "Code: " << ex.code << std::endl << "MSG: " << ex.msg << std::endl;
		}
		return false;
}

bool EmployeeManager::getPwdToken(IDType eID, BYTE * databaseTokenHash, OTL_BIGINT& tokenTime, DeviceID devID, DBManager * dbManager)
{
		std::string query = "SELECT tokenHash, tokenTime FROM PwdTokens WHERE eID = :f1<int> AND deviceID = :f2<int>";
		try {
				otl_stream otlStream(OTL_BUFFER_SIZE, query.c_str(), *dbManager->getConnection());
				otlStream << (int)eID;
				otlStream << (int)devID;
				if (!otlStream.eof()) {
						CryptoManager::InputBytes(otlStream, databaseTokenHash, TOKEN_SIZE);
						otlStream >> tokenTime;
						return true;
				}
		}
		catch (otl_exception ex)
		{
				std::cerr << "Code: " << ex.code << std::endl << "MSG: " << ex.msg << std::endl;
		}
		return false;
}

void EmployeeManager::loginClient(BB_Client * bbClient, IDType eID)
{
		bbClient->setEmpID(eID);
		employees.emplace(std::make_pair(eID, bbClient));
}

EmployeeManager::~EmployeeManager()
{
		
}
