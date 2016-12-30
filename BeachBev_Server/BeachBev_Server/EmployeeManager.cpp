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

EmployeeManager::EmployeeManager(BB_Server* bbServer)
		:PKeyOwner(bbServer->getPacketManager()), bbServer(bbServer)
{
		addKey(new PKey("D0", this, &EmployeeManager::keyD0));
		addKey(new PKey("E0", this, &EmployeeManager::keyE0));
		emailManager = new EmailManager(bbServer, this);
}

void EmployeeManager::keyD0(boost::shared_ptr<IPacket> iPack)
{
		ProtobufPackets::PackD0 packD0;
		packD0.ParseFromString(*iPack->getData());
		ProtobufPackets::PackD1 packD1;
		packD1.set_success(false);
		BB_Client* sender = (BB_Client*)(bbServer->getClientManager()->getClient(iPack->getSentFromID()));
		if (sender != nullptr) {
				DBManager* dbManager = sender->getDBManager();
				IDType eID = nameToEID(packD0.username(), dbManager);
				if (eID != 0)
				{
						if (employees.find(eID) == employees.end())
						{
								if (checkPwd(eID, packD0.pwd(), dbManager))
								{
										otl_stream otlStream(50, "SELECT aState FROM Employees WHERE eID = :f1<int>", *dbManager->getConnection());
										otlStream << (int)eID;
										int aState = 0;
										otlStream >> aState;
										if (aState != 0)
										{
												BYTE genToken[TOKEN_SIZE];
												CryptoManager::GenerateRandomData(genToken, TOKEN_SIZE);
												if (setToken(eID, dbManager, genToken)) {
														sender->setEmpID(eID);
														employees.emplace(eID, sender);
														packD1.set_success(true);
														packD1.set_token(std::string(reinterpret_cast<char*>(genToken), TOKEN_SIZE));
														DebugManager::PrintDebug("Employee logged in");
												}
												else
												{
														packD1.set_error("Could not set token");
												}
										}
										else
										{
												packD1.set_error("Waiting for Approval");
										}
								}
								else
								{
										packD1.set_error("Incorrect Password");
								}
						}
						else
						{
								packD1.set_error("Someone is already logged in on this account");
						}
				}
				else
				{
						packD1.set_error("Nonexistant Username");
				}
		}
		else
		{
				packD1.set_error("Could not link client to database connection");
		}
		boost::shared_ptr<OPacket> oPackD1 = boost::make_shared<WSOPacket>("D1", 0, iPack->getSentFromID());
		oPackD1->setData(boost::make_shared<std::string>(packD1.SerializeAsString()));
		bbServer->getClientManager()->send(oPackD1);
}

void EmployeeManager::keyE0(boost::shared_ptr<IPacket> iPack)
{
		BB_Client* sender = (BB_Client*)bbServer->getClientManager()->getClient(iPack->getSentFromID());
		if (sender != nullptr) {
				DBManager* dbManager = sender->getDBManager();
				ProtobufPackets::PackE0 packE0;
				packE0.ParseFromString(*iPack->getData());
				ProtobufPackets::PackE1 packE1;
				packE1.set_success(false);
				if (CmdInjectSafe(packE0.email(), packE0.email().size())) {
						IDType eID = nameToEID(packE0.username(), dbManager);
						if (eID == 0)
						{
								IDType nextID = getNextEID(dbManager);
								if (nextID != 0)
								{
										if (addEmployeeToDatabase(nextID, dbManager, packE0.username(), packE0.pwd(), packE0.email())) {
												if (emailManager->sendEmailVerification(nextID, dbManager, packE0.email()))
												{
														std::string urlEncodedCreationToken;
														if (generateCreationToken(nextID, dbManager, urlEncodedCreationToken));
														{
																packE1.set_success(true);
																packE1.set_msg("Verify Email");
																packE1.set_creationtoken(urlEncodedCreationToken );
																DebugManager::PrintDebug("Employee account added");
														}
												}
										}
										else
										{
												packE1.set_msg("An error occured: Failed to add to database");
										}
								}
								else
								{
										packE1.set_msg("An error occured: Could not assign empID");
								}
						}
						else
						{
								packE1.set_msg("The username already exists");
						}
				}
				else
				{
						packE1.set_msg("Email cannot contain \' or \" characters");
				}
				boost::shared_ptr<WSOPacket> oPackE1 = boost::make_shared<WSOPacket>("E1", 0, iPack->getSentFromID());
				oPackE1->setData(boost::make_shared<std::string>(packE1.SerializeAsString()));
				bbServer->getClientManager()->send(oPackE1);
		}
}

bool EmployeeManager::generateCreationToken(IDType eID, DBManager* dbManager, std::string& urlEncodedCreationToken) {
		BYTE creationToken[EmailManager::CREATION_TOKEN_SIZE];
		CryptoManager::GenerateRandomData(creationToken, EmailManager::CREATION_TOKEN_SIZE);
		CryptoManager::UrlEncode(urlEncodedCreationToken, creationToken, EmailManager::CREATION_TOKEN_SIZE);
		BYTE creationTokenHash[EmailManager::CREATION_HASH_SIZE];
		CryptoManager::GenerateHash(creationTokenHash, EmailManager::CREATION_HASH_SIZE, creationToken, EmailManager::CREATION_TOKEN_SIZE);
		return emailManager->setCreationTokenHash(eID, dbManager, creationTokenHash);
}

bool EmployeeManager::addEmployeeToDatabase(IDType eID, DBManager* dbManager, const std::string & name, const std::string & pwdStr, const std::string & email)
{
		std::vector <BYTE> pwd(pwdStr.begin(), pwdStr.end());
		BYTE genSalt[SALT_SIZE];
		CryptoManager::GenerateRandomData(genSalt, SALT_SIZE);
		BYTE genHash[HASH_SIZE];
		CryptoManager::GenerateHash(genHash, HASH_SIZE, pwd.data(), pwdStr.size(), genSalt, SALT_SIZE);
		std::string query = "INSERT INTO Employees (eID, name, email, aState, hashPwd, salt)\
						VALUES (:f1<int>, :f2<char[";
		query += std::to_string(NAME_SIZE);
		query += "]>, :f3<char[";
		query += std::to_string(EMAIL_SIZE);
		query += "]>, :f4<int>, :f5<raw[";
		query += std::to_string(HASH_SIZE);
		query += "]>, :f6<raw[";
		query += std::to_string(SALT_SIZE);
		query += "]>)";
		try {
				otl_stream otlStream(50, query.c_str(), *dbManager->getConnection());
				otlStream << (int)eID;
				otlStream << name;
				otlStream << email;
				otlStream << INITIAL_A_STATE;
				otl_long_string genHashStr(HASH_SIZE);
				for (int i = 0; i < HASH_SIZE; i++) {
						genHashStr[i] = genHash[i];
				}
				genHashStr.set_len(HASH_SIZE);
				otlStream << genHashStr;
				otl_long_string genSaltStr(SALT_SIZE);
				for (int i = 0; i < SALT_SIZE; i++) {
						genSaltStr[i] = genSalt[i];
				}
				genSaltStr.set_len(SALT_SIZE);
				otlStream << genSaltStr;
				return true;
		}
		catch (otl_exception ex)
		{
				std::cerr << "Code: " << ex.code << std::endl << " MSG: " << ex.msg << " VAR INFO: " << ex.var_info << std::endl;
		}
		return false;
}

bool EmployeeManager::checkPwd(IDType eID, const std::string & pwdStr, DBManager* dbManager)
{
		try
		{
				std::vector <BYTE> pwd(pwdStr.begin(), pwdStr.end());
				BYTE dbHash[HASH_SIZE];
				BYTE dbSalt[SALT_SIZE];
				getPwdData(eID, dbManager, dbHash, dbSalt);
				BYTE genHash[HASH_SIZE];
				CryptoManager::GenerateHash(genHash, HASH_SIZE, pwd.data(), pwdStr.size(), dbSalt, SALT_SIZE);
				for (int i = 0; i < HASH_SIZE; i++)
				{
						if (genHash[i] != dbHash[i])
						{
								return false;
						}
				}
				return true;
		}
		catch (otl_exception ex)
		{
				std::cerr << "Code: " << ex.code << std::endl << " MSG: " << ex.msg << " VAR INFO: " << ex.var_info << std::endl;
		}
		return false;
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

bool EmployeeManager::setToken(IDType eID, DBManager * dbManager, BYTE * token)
{
		std::string query = "UPDATE Employees SET token=:f1<char[";
		query += std::to_string(TOKEN_SIZE);
		query += "]> WHERE eID=:f2<int>";
		try {
				otl_stream otlStream(50, query.c_str(), *dbManager->getConnection());
				otlStream << token, eID;
				return true;
		}
		catch (otl_exception ex)
		{
				std::cerr << "Code: " << ex.code << std::endl << "MSG: " << ex.msg << std::endl;
		}
		return false;
}

IDType EmployeeManager::nameToEID(const std::string& name, DBManager* dbManager)
{
		std::string query = "SELECT * FROM Employees WHERE name = :f1<char[";
		query += std::to_string((int)NAME_SIZE);
		query += "]>";
		try
		{
				otl_stream otlStream(1, query.c_str(), *dbManager->getConnection());
				otlStream << name;
				dbManager->getConnection()->commit();
				int eID = 0;
				otlStream >> eID;
				return (IDType)eID;
		}
		catch (otl_exception ex)
		{
				std::cerr << "Code: " << ex.code << std::endl << "MSG: " << ex.msg << std::endl;
		}
		return 0;
}

IDType EmployeeManager::getNextEID(DBManager* dbManager)
{
		try
		{
				std::string query;
#ifdef _WIN32
				query = "SELECT top 1 * FROM Employees ORDER BY eID desc";
#else
				query = "SELECT * FROM Employees ORDER BY eID desc limit 1";
#endif
				otl_stream otlStream(50, query.c_str(), *dbManager->getConnection());
				dbManager->getConnection()->commit();
				int eID = 0;
				if (!otlStream.eof())
				{
						otlStream >> eID;
				}
				return (IDType)(eID + 1);
		}
		catch (otl_exception ex)
		{
				std::cerr << "Code: " << ex.code << std::endl << "MSG: " << ex.msg << std::endl;
		}
		return 0;
}

bool EmployeeManager::getPwdData(IDType eID, DBManager * dbManager, BYTE * hash, BYTE * salt)
{
		try {
				otl_stream otlStream(50, "SELECT hashPwd, salt FROM dbo.Employees WHERE eID = :f1<int>", *dbManager->getConnection());
				otlStream << (int)eID;
				if (!otlStream.eof()) {
						otlStream >> hash;
						otlStream >> salt;
						return true;
				}
		}
		catch (otl_exception ex) {
				std::cerr << "getSalt failed\nCode: " << ex.code << std::endl << " MSG: " << ex.msg << " VAR INFO: " << ex.var_info << std::endl;
		}
		return false;
}

EmployeeManager::~EmployeeManager()
{
}
