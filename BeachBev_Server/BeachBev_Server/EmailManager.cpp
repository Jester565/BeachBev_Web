#include "EmailManager.h"
#include "DBManager.h"
#include "CryptoManager.h"
#include "BB_Server.h"
#include <cryptopp/base64.h>

const std::string EmailManager::EMAIL_CONFIRM_URL = "https://localhost/email_confirm.html";
const std::vector <BYTE> EmailManager::EMAIL_SALT(5, (BYTE)0);

EmailManager::EmailManager()
{
}

bool EmailManager::sendEmailVerification(IDType eID, DBManager* dbManager, const std::string& emailAddress) {
		BYTE emailToken[EMAIL_TOKEN_SIZE];
		CryptoManager::GenerateRandomData(emailToken, EMAIL_TOKEN_SIZE);
		std::string emailTokenEncoded;

		BYTE emailTokenHash[EMAIL_HASH_SIZE];
		CryptoManager::GenerateHash(emailTokenHash, EMAIL_HASH_SIZE, emailToken, EMAIL_TOKEN_SIZE, EMAIL_SALT.data(), EMAIL_SALT.size());
		setEmailTokenHash(eID, dbManager, emailTokenHash);

		CryptoManager::UrlEncode(emailTokenEncoded, emailToken, EMAIL_TOKEN_SIZE);
		std::string emailURL = EMAIL_CONFIRM_URL + "?" + emailTokenEncoded;
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
		}
		catch (otl_exception ex)
		{
				std::cerr << "Code: " << ex.code << std::endl << " MSG: " << ex.msg << " VAR INFO: " << ex.var_info << std::endl;
		}
}

bool EmailManager::sendEmail(const std::string & sendToAddress, const std::string & senderAddress, const std::string& senderName, const std::string & subject, const std::string & body)
{
		std::string cmd = "echo \"";
		cmd += body;
		cmd += "\" | mail -a \"From: ";
		cmd += senderName;
		cmd += " <";
		cmd += senderAddress;
		cmd += ">\" -a \"Subject: ";
		cmd += subject;
		cmd += "\" -a \"X-Custom-Header: yes\" ";
		cmd += sendToAddress;
		system(cmd.c_str());
}


EmailManager::~EmailManager()
{
}
