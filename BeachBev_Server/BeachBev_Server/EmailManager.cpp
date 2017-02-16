#include "EmailManager.h"
#include "DBManager.h"
#include "CryptoManager.h"
#include "BB_Client.h"
#include "EmployeeManager.h"
#include "Packets/BBPacks.pb.h"
#include <WSIPacket.h>
#include <WSOPacket.h>
#include <ClientManager.h>
#include <cryptopp/base64.h>

const std::string EmailManager::EMAIL_CONFIRM_URL = "https://beachbevs.com/email_confirm.html";
const std::string EmailManager::EMAIL_HTML_DIR = "/home/ubuntu/BeachBev_Web/";

EmailManager::EmailManager(BB_Server* bbServer, EmployeeManager* employeeManager)
		:PKeyOwner(bbServer->getPacketManager()), bbServer(bbServer)
{
		addKey(new PKey("B0", this, &EmailManager::handleB0));
		addKey(new PKey("B2", this, &EmailManager::handleB2));
		addKey(new PKey("B4", this, &EmailManager::handleB4));
}

void EmailManager::handleB0(boost::shared_ptr<IPacket> iPack)
{
		ProtobufPackets::PackB0 packB0;
		packB0.ParseFromString(*iPack->getData());
		ProtobufPackets::PackB1 replyPacket;
		replyPacket.set_success(false);
		BB_Client* sender = (BB_Client*)(bbServer->getClientManager()->getClient(iPack->getSentFromID()));
		if (sender == nullptr) {
				return;
		}
		DBManager* dbManager = sender->getDBManager();
		if (sender->getEmpID() > 0) {
				IDType emailEID = verifiedEmailToEID(packB0.email(), dbManager);
				if (emailEID == 0) {
						emailEID = unverifiedEmailToEID(packB0.email(), dbManager);
						if (emailEID == 0 || emailEID == sender->getEmpID()) {
								std::string urlEncodedEmailToken;
								if (setUnverifiedEmail(sender->getEmpID(), packB0.email(), urlEncodedEmailToken, dbManager)) {
										sendVerificationEmail(packB0.email(), urlEncodedEmailToken);
										replyPacket.set_success(true);
										replyPacket.set_msg("Email sent");
								}
								else
								{
										replyPacket.set_msg("Failed to set unverified email");
								}
						}
						else
						{
								replyPacket.set_msg("This email is already used");
						}
				}
				else
				{
						replyPacket.set_msg("This email is already used");
				}
		}
		else
		{
				replyPacket.set_msg("Not logged in");
		}
		boost::shared_ptr<OPacket> oPack = boost::make_shared<WSOPacket>("B1");
		oPack->setSenderID(0);
		oPack->setData(boost::make_shared<std::string>(replyPacket.SerializeAsString()));
		bbServer->getClientManager()->send(oPack, sender);
}

void EmailManager::handleB2(boost::shared_ptr<IPacket> iPack)
{
		ProtobufPackets::PackB2 packB2;
		packB2.ParseFromString(*iPack->getData());
		ProtobufPackets::PackB3 replyPacket;
		replyPacket.set_success(false);
		BB_Client* sender = (BB_Client*)(bbServer->getClientManager()->getClient(iPack->getSentFromID()));
		if (sender == nullptr) {
				return;
		}
		DBManager* dbManager = sender->getDBManager();
		if (sender->getEmpID() > 0) {
				BYTE dbEmailTokenHash[TOKEN_SIZE];
				OTL_BIGINT tokenTime;
				if (getEmailToken(sender->getEmpID(), dbEmailTokenHash, tokenTime, dbManager)) {
						if (EmployeeManager::CheckInTimeRange(tokenTime, MAX_TOKEN_HOURS)) {
								std::vector<BYTE> packEmailToken;
								packEmailToken.reserve(TOKEN_SIZE);
								CryptoManager::UrlDecode(packEmailToken, packB2.emailtoken());
								BYTE packEmailTokenHash[TOKEN_SIZE];
								CryptoManager::GenerateHash(packEmailTokenHash, TOKEN_SIZE, packEmailToken.data(), packEmailToken.size());
								bool match = true;
								for (int i = 0; i < TOKEN_SIZE; i++) {
										if (dbEmailTokenHash[i] != packEmailTokenHash[i]) {
												match = false;
										}
								}
								if (match) {
										std::string prevEmail;
										if (getVerifiedEmail(sender->getEmpID(), prevEmail, dbManager)) {
												sendChangeEmail(prevEmail);
										}
										verifyEmail(sender->getEmpID(), dbManager);
										replyPacket.set_success(true);
										replyPacket.set_msg("Email verified");
								}
								else
								{
										replyPacket.set_msg("Invalid token");
								}
						}
						else
						{
								replyPacket.set_msg("Token expired");
						}
				}
				else
				{
						replyPacket.set_msg("Failed to get token from database");
				}
		}
		else
		{
				replyPacket.set_msg("Not logged in");
		}
		boost::shared_ptr<OPacket> oPack = boost::make_shared<WSOPacket>("B3");
		oPack->setSenderID(0);
		oPack->setData(boost::make_shared<std::string>(replyPacket.SerializeAsString()));
		bbServer->getClientManager()->send(oPack, sender);
}

void EmailManager::handleB4(boost::shared_ptr<IPacket> iPack) {
		ProtobufPackets::PackB5 replyPacket;
		BB_Client* sender = (BB_Client*)(bbServer->getClientManager()->getClient(iPack->getSentFromID()));
		if (sender == nullptr) {
				return;
		}
		DBManager* dbManager = sender->getDBManager();
		if (sender->getEmpID() > 0) {
				std::string verifiedEmail;
				getVerifiedEmail(sender->getEmpID(), verifiedEmail, dbManager);
				std::string unverifiedEmail;
				getUnverifiedEmail(sender->getEmpID(), unverifiedEmail, dbManager);
				replyPacket.set_verifiedemail(verifiedEmail);
				replyPacket.set_unverifiedemail(unverifiedEmail);
		}
		boost::shared_ptr<OPacket> oPack = boost::make_shared<WSOPacket>("B5");
		oPack->setSenderID(0);
		oPack->setData(boost::make_shared<std::string>(replyPacket.SerializeAsString()));
		bbServer->getClientManager()->send(oPack, sender);
}

bool EmailManager::setUnverifiedEmail(IDType eID, const std::string & email, std::string& urlEncodedEmailToken, DBManager * dbManager)
{
		BYTE genToken[TOKEN_SIZE];
		CryptoManager::GenerateRandomData(genToken, TOKEN_SIZE);
		BYTE genTokenHash[TOKEN_SIZE];
		CryptoManager::GenerateHash(genTokenHash, TOKEN_SIZE, genToken, TOKEN_SIZE);
		std::string query = "REPLACE INTO UnverifiedEmails VALUES (:f1<int>, :f2<char[";
		query += std::to_string(EMAIL_SIZE);
		query += "]>, :f3<raw[";
		query += std::to_string(TOKEN_SIZE);
		query += "]>, :f4<bigint>)";
		try {
				otl_stream otlStream(OTL_BUFFER_SIZE, query.c_str(), *dbManager->getConnection());
				otlStream << (int)eID;
				otlStream << email;
				CryptoManager::OutputBytes(otlStream, genTokenHash, TOKEN_SIZE);
				otlStream << (OTL_BIGINT)(std::time(NULL));
		}
		catch (otl_exception ex)
		{
				std::cerr << "Code: " << ex.code << std::endl << "MSG: " << ex.msg << std::endl;
		}
		CryptoManager::UrlEncode(urlEncodedEmailToken, genToken, TOKEN_SIZE);
		return true;
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

IDType EmailManager::emailToEID(const std::string & email, DBManager * dbManager)
{
		IDType eID = verifiedEmailToEID(email, dbManager);
		if (eID == 0) {
				return unverifiedEmailToEID(email, dbManager);
		}
		return eID;
}

IDType EmailManager::verifiedEmailToEID(const std::string & email, DBManager * dbManager)
{
		IDType eID = 0;
		std::string query = "SELECT eID FROM Employees WHERE email=:f1<char[";
		query += std::to_string((int)EMAIL_SIZE);
		query += "]>";
		try
		{
				otl_stream otlStream(OTL_BUFFER_SIZE, query.c_str(), *dbManager->getConnection());
				otlStream << email;
				if (!otlStream.eof()) {
						int eIDInt = 0;
						otlStream >> eIDInt;
						eID = eIDInt;
				}
		}
		catch (otl_exception ex)
		{
				std::cerr << "Code: " << ex.code << std::endl << "MSG: " << ex.msg << std::endl;
		}
		return eID;
}

IDType EmailManager::unverifiedEmailToEID(const std::string & email, DBManager * dbManager)
{
		IDType eID = 0;
		std::string query = "SELECT * FROM UnverifiedEmails WHERE email=:f1<char[";
		query += std::to_string((int)EMAIL_SIZE);
		query += "]>";
		try
		{
				otl_stream otlStream(OTL_BUFFER_SIZE, query.c_str(), *dbManager->getConnection());
				otlStream << email;
				if (!otlStream.eof()) {
						int eIDInt = 0;
						otlStream >> eIDInt;
						eID = eIDInt;
				}
		}
		catch (otl_exception ex)
		{
				std::cerr << "Code: " << ex.code << std::endl << "MSG: " << ex.msg << std::endl;
		}
		return eID;
}

EmailManager::~EmailManager()
{
}

bool EmailManager::verifyEmail(IDType eID, DBManager * dbManager)
{
		std::string unverifiedEmail;
		if (!getUnverifiedEmail(eID, unverifiedEmail, dbManager)) {
				return false;
		}
		removeUnverifiedEmail(eID, dbManager);
		std::string query = "UPDATE Employees SET email=:f1<char[";
		query += std::to_string(EMAIL_SIZE);
		query += "]> WHERE eID=:f2<int>";
		try {
				otl_stream otlStream(OTL_BUFFER_SIZE, query.c_str(), *dbManager->getConnection());
				otlStream << unverifiedEmail;
				otlStream << (int)eID;
				return true;
		}
		catch (otl_exception ex)
		{
				std::cerr << "Code: " << ex.code << std::endl << "MSG: " << ex.msg << std::endl;
		}
		return false;
}

bool EmailManager::removeUnverifiedEmail(IDType eID, DBManager * dbManager)
{
		std::string query = "DELETE FROM UnverifiedEmails WHERE eID = :f1<int>";
		try {
				otl_stream otlStream(OTL_BUFFER_SIZE, query.c_str(), *dbManager->getConnection());
				otlStream << (int)eID;
				return true;
		}
		catch (otl_exception ex)
		{
				std::cerr << "Code: " << ex.code << std::endl << "MSG: " << ex.msg << std::endl;
		}
		return false;
}

bool EmailManager::getEmailToken(IDType eID, BYTE * dbEmailTokenHash, OTL_BIGINT& tokenTime, DBManager * dbManager)
{
		std::string query = "SELECT tokenHash, tokenTime FROM UnverifiedEmails WHERE eID=:f1<int>";
		try {
				otl_stream otlStream(OTL_BUFFER_SIZE, query.c_str(), *dbManager->getConnection());
				otlStream << (int)eID;
				if (!otlStream.eof()) {
						CryptoManager::InputBytes(otlStream, dbEmailTokenHash, TOKEN_SIZE);
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

bool EmailManager::getVerifiedEmail(IDType eID, std::string & email, DBManager * dbManager)
{
		std::string query = "SELECT email FROM Employees WHERE eID=:f1<int>";
		try {
				otl_stream otlStream(OTL_BUFFER_SIZE, query.c_str(), *dbManager->getConnection());
				otlStream << (int)eID;
				if (!otlStream.eof()) {
						otlStream >> email;
						return true;
				}
		}
		catch (otl_exception ex)
		{
				std::cerr << "Code: " << ex.code << std::endl << "MSG: " << ex.msg << std::endl;
		}
		return false;
}

bool EmailManager::getUnverifiedEmail(IDType eID, std::string & email, DBManager * dbManager)
{
		std::string query = "SELECT email FROM UnverifiedEmails WHERE eID=:f1<int>";
		try {
				otl_stream otlStream(OTL_BUFFER_SIZE, query.c_str(), *dbManager->getConnection());
				otlStream << (int)eID;
				if (!otlStream.eof()) {
						otlStream >> email;
						return true;
				}
		}
		catch (otl_exception ex)
		{
				std::cerr << "Code: " << ex.code << std::endl << "MSG: " << ex.msg << std::endl;
		}
		return false;
}

bool EmailManager::sendVerificationEmail(const std::string& sendToAddress, const std::string& urlEncodedEmailToken)
{
		std::string emailURL = EMAIL_CONFIRM_URL + "?" + urlEncodedEmailToken;

		std::cout << "EMAIL URL: " << emailURL << std::endl;

		std::string bodyCmd = "(cat ";
		bodyCmd += EMAIL_HTML_DIR + "emailPt1.html";
		bodyCmd += "; echo -n \"";
		bodyCmd += emailURL;
		bodyCmd += "\"; cat ";
		bodyCmd += EMAIL_HTML_DIR + "emailPt2.html";
		bodyCmd += ")";
		return sendEmail(sendToAddress, "management@beachbevs.com", "BeachBevs", "Email Verification", bodyCmd, true);
}

bool EmailManager::sendChangeEmail(const std::string& sendToAddress)
{
		std::string bodyCmd = "(cat ";
		bodyCmd += EMAIL_HTML_DIR + "emailChange.html";
		bodyCmd += ")";
		return sendEmail(sendToAddress, "management@beachbevs.com", "BeachBevs", "Email Changed", bodyCmd, true);
}