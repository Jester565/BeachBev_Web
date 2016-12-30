#pragma once
#include <PKeyOwner.h>
#include "BB_Server.h"
#include <boost/shared_ptr.hpp>
#include <vector>

class EmailManager : public PKeyOwner
{
public:
		static const int EMAIL_TOKEN_SIZE = 64;
		static const int EMAIL_HASH_SIZE = 64;
		static const int CREATION_TOKEN_SIZE = 64;
		static const int CREATION_HASH_SIZE = 64;
		static const std::string EMAIL_CONFIRM_URL;
		static const std::string EMAIL_HTML_DIR;
		static const std::string EMAIL_VERIFY_BODY_CMD;
		static const OTL_BIGINT EMAIL_EXPIRE_TIME = 7200;

		EmailManager(BB_Server* bbServer, EmployeeManager* employeeManager);

		bool sendEmailVerification(IDType eID, DBManager* dbManager, const std::string& emailAddress);

		bool setCreationTokenHash(IDType id, DBManager* dbManager, BYTE* creationTokenHash);

		bool setEmailTokenHash(IDType eID, DBManager* dbManager, BYTE* emailTokenHash);

		bool sendEmail(const std::string& sendToAddress, const std::string& senderAddress, const std::string& senderName, const std::string& subject, const std::string& bodyCmd, bool isHTML = false);

		void keyI0(boost::shared_ptr <IPacket> iPack);

		void keyI1(boost::shared_ptr <IPacket> iPack);

		~EmailManager();

private:
		BB_Server* bbServer;

		EmployeeManager* employeeManager;
};
