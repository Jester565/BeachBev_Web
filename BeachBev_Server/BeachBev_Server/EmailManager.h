#pragma once
#include <PKeyOwner.h>
#include "BB_Server.h"
#include <boost/shared_ptr.hpp>
#include <vector>

class EmailManager : public PKeyOwner
{
public:
		static const std::string EMAIL_CONFIRM_URL;
		static const std::string EMAIL_HTML_DIR;
		static const std::string EMAIL_VERIFY_BODY_CMD;

		EmailManager(BB_Server* bbServer, EmployeeManager* employeeManager);

		void handleB0(boost::shared_ptr<IPacket> iPack);

		void handleB2(boost::shared_ptr<IPacket> iPack);

		void handleB4(boost::shared_ptr<IPacket> iPack);

		bool setUnverifiedEmail(IDType eID, const std::string& email, std::string& urlEncodedEmailToken, DBManager* dbManager);

		bool sendEmail(const std::string& sendToAddress, const std::string& senderAddress, const std::string& senderName, const std::string& subject, const std::string& bodyCmd, bool isHTML = false);

		IDType emailToEID(const std::string& email, DBManager* dbManager);
		IDType verifiedEmailToEID(const std::string& email, DBManager* dbManager);
		IDType unverifiedEmailToEID(const std::string& email, DBManager* dbManager);

		bool sendVerificationEmail(const std::string& sendToAddress, const std::string& urlEncodedEmailToken);
		~EmailManager();

private:
		bool verifyEmail(IDType eID, DBManager* dbManager);
		bool removeUnverifiedEmail(IDType eID, DBManager* dbManager);
		bool getEmailToken(IDType eID, BYTE* dbEmailTokenHash, OTL_BIGINT& tokenTime, DBManager* dbManager);
		bool getVerifiedEmail(IDType eID, std::string& email, DBManager* dbManager);
		bool getUnverifiedEmail(IDType eID, std::string& email, DBManager* dbManager);
		BB_Server* bbServer;

		EmployeeManager* employeeManager;
};
