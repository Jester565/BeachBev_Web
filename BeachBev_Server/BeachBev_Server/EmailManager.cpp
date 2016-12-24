#include "EmailManager.h"
#include "DBManager.h"
#include "CryptoManager.h"
#include "BB_Client.h"
#include "Packets/BBPacks.pb.h"
#include <WSIPacket.h>
#include <WSOPacket.h>
#include <ClientManager.h>
#include <cryptopp/base64.h>

const std::string EmailManager::EMAIL_CONFIRM_URL = "https://beachbevs.com/email_confirm.html";
const std::string EmailManager::EMAIL_HTML_DIR = "/home/ubuntu/BeachBev_Web/";

EmailManager::EmailManager(BB_Server* bbServer)
		:PKeyOwner(bbServer->getPacketManager()), bbServer(bbServer)
{
		addKey(new PKey("I0", this, &EmailManager::keyI0));
}

bool EmailManager::sendEmailVerification(IDType eID, DBManager* dbManager, const std::string& emailAddress) {
		BYTE emailToken[EMAIL_TOKEN_SIZE];
		CryptoManager::GenerateRandomData(emailToken, EMAIL_TOKEN_SIZE);
		std::string emailTokenEncoded;

		BYTE emailTokenHash[EMAIL_HASH_SIZE];
		CryptoManager::GenerateHash(emailTokenHash, EMAIL_HASH_SIZE, emailToken, EMAIL_TOKEN_SIZE);
		if (!setEmailTokenHash(eID, dbManager, emailTokenHash)) {
				return false;
		}

		std::cout << "Email Token Non-URL: ";
		for (int i = 0; i < EMAIL_TOKEN_SIZE; i++) {
				std::cout << (int)emailToken[i] << " ";
		}
		std::cout << std::endl << std::endl;
		CryptoManager::UrlEncode(emailTokenEncoded, emailToken, EMAIL_TOKEN_SIZE);
		std::cout << "Email Token URL: ";
		std::cout << "SIZE: " << emailTokenEncoded.size() << " ";
		for (int i = 0; i < emailTokenEncoded.size(); i++) {
				std::cout << (int)emailTokenEncoded[i] << " ";
		}
		std::cout << std::endl << std::endl;
		std::string emailURL = EMAIL_CONFIRM_URL + "?" + emailTokenEncoded;

		std::string bodyCmd = "(cat ";
		bodyCmd += EMAIL_HTML_DIR + "emailPt1.html";
		bodyCmd += "; echo -n \"";
		bodyCmd += emailURL;
		bodyCmd += "\"; cat ";
		bodyCmd += EMAIL_HTML_DIR + "emailPt2.html";
		bodyCmd += ")";
		return sendEmail(emailAddress, "management@beachbevs.com", "BeachBevs", "Email Verification",	bodyCmd, true);
}

bool EmailManager::setCreationTokenHash(IDType eID, DBManager * dbManager, BYTE * creationTokenHash)
{
		std::string query = "UPDATE Employees SET creationToken=:f1<raw[";
		query += std::to_string(CREATION_HASH_SIZE);
		query += "]> WHERE eID=:f2<int>";
		try {
				otl_stream otlStream(50, query.c_str(), *dbManager->getConnection());
				otl_long_string creationTokenHashStr(CREATION_HASH_SIZE);
				for (int i = 0; i < CREATION_HASH_SIZE; i++) {
						creationTokenHashStr[i] = creationTokenHash[i];
				}
				creationTokenHashStr.set_len(CREATION_HASH_SIZE);
				otlStream << creationTokenHashStr;
				otlStream << (int)eID;
				return true;
		}
		catch (otl_exception ex)
		{
				std::cerr << "Code: " << ex.code << std::endl << " MSG: " << ex.msg << " VAR INFO: " << ex.var_info << std::endl;
		}
		return false;
}

bool EmailManager::setEmailTokenHash(IDType eID, DBManager * dbManager, BYTE * emailTokenHash)
{
		std::string query = "UPDATE Employees SET emailToken=:f1<raw[";
		query += std::to_string(EMAIL_HASH_SIZE);
		query += "]>, emailTokenTime=:f2<bigint> WHERE eID=:f3<int>";
		try {
				otl_stream otlStream(50, query.c_str(), *dbManager->getConnection());
				otl_long_string emailTokenHashStr(EMAIL_HASH_SIZE);
				for (int i = 0; i < EMAIL_HASH_SIZE; i++) {
						emailTokenHashStr[i] = emailTokenHash[i];
				}
				emailTokenHashStr.set_len(EMAIL_HASH_SIZE);
				otlStream << emailTokenHashStr;
				otlStream << static_cast<OTL_BIGINT>(std::time(nullptr));
				otlStream << (int)eID;
				return true;
		}
		catch (otl_exception ex)
		{
				std::cerr << "Code: " << ex.code << std::endl << " MSG: " << ex.msg << " VAR INFO: " << ex.var_info << std::endl;
		}
		return false;
}

bool EmailManager::sendEmail(const std::string & sendToAddress, const std::string & senderAddress, const std::string& senderName, const std::string & subject, const std::string & bodyCmd, bool isHTML)
{
		std::string cmd = bodyCmd;
		cmd += " | mail -a \"From: ";
		cmd += senderName;
		cmd += " <";
		cmd += senderAddress;
		cmd += ">\" -a \"Subject: ";
		cmd += subject;
		cmd += "\" -a \"X-Custom-Header: yes\" ";
		if (isHTML) {
				cmd += "-a \"Content-type: text/html;\" ";
		}
		cmd += sendToAddress;
		system(cmd.c_str());
		return true;
}

void EmailManager::keyI0(boost::shared_ptr<IPacket> iPack)
{
		ProtobufPackets::PackI0 packI0;
		packI0.ParseFromString(*iPack->getData());
		
		std::vector <BYTE> emailTokenDecoded;
		std::cout << "Email Token URL: ";
		std::cout << " SIZE: " << packI0.emailtoken().size() + " ";
		for (int i = 0; i < packI0.emailtoken().size(); i++) {
				std::cout << (int)packI0.emailtoken()[i] << " ";
		}
		std::cout << std::endl << std::endl;
		CryptoManager::UrlDecode(emailTokenDecoded, packI0.emailtoken());
		std::cout << "Email Token Non-URL: ";
		for (int i = 0; i < EMAIL_TOKEN_SIZE; i++) {
				std::cout << (int)emailTokenDecoded[i] << " ";
		}
		BYTE emailTokenHash[EMAIL_HASH_SIZE];
		CryptoManager::GenerateHash(emailTokenHash, EMAIL_HASH_SIZE, emailTokenDecoded.data(), EMAIL_TOKEN_SIZE);
		
		std::vector <BYTE> creationTokenDecoded;
		CryptoManager::UrlDecode(creationTokenDecoded, packI0.creationtoken());
		BYTE creationTokenHash[CREATION_HASH_SIZE];
		CryptoManager::GenerateHash(creationTokenHash, CREATION_HASH_SIZE, creationTokenDecoded.data(), CREATION_TOKEN_SIZE);
		
		BB_Client* sender = (BB_Client*)bbServer->getClientManager()->getClient(iPack->getSentFromID());
		if (sender == nullptr)
		{
				std::cerr << "packI0 could not identify the sender" << std::endl;
		}
		else
		{
				ProtobufPackets::PackI2 packI2;
				packI2.set_success(false);
				packI2.set_msg("An unknown error occured");

				DBManager* dbManager = sender->getDBManager();
				std::string query = "SELECT name FROM Employees WHERE emailToken=:f1<raw[";
				query += std::to_string(EMAIL_HASH_SIZE);
				query += "]>";
				try {
						otl_stream otlStream(50, query.c_str(), *dbManager->getConnection());
						otl_long_string emailTokenHashStr(EMAIL_HASH_SIZE);
						for (int i = 0; i < EMAIL_HASH_SIZE; i++) {
								emailTokenHashStr[i] = emailTokenHash[i];
						}
						emailTokenHashStr.set_len(EMAIL_HASH_SIZE);
						otlStream << emailTokenHashStr;
						if (!otlStream.eof()) {
								std::string name;
								otlStream >> name;
								packI2.set_success(true);
								packI2.set_msg("Thank you for confirming your email " + name);
						}
						else
						{
								packI2.set_msg("Could not link tokens to account");
						}
				}
				catch (otl_exception ex)
				{
						std::cerr << "Code: " << ex.code << std::endl << " MSG: " << ex.msg << " VAR INFO: " << ex.var_info << std::endl;
				}
				boost::shared_ptr<WSOPacket> oPackI2 = boost::make_shared<WSOPacket>("E1", 0, iPack->getSentFromID());
				oPackI2->setData(boost::make_shared<std::string>(packI2.SerializeAsString()));
				bbServer->getClientManager()->send(oPackI2);
		}
}

EmailManager::~EmailManager()
{
}
