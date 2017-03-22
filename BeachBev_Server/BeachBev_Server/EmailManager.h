#pragma once
#include "stdafx.h"
#include <PKeyOwner.h>
#include "BB_Server.h"
#include <boost/shared_ptr.hpp>
#include <vector>
#include <aws/email/SESClient.h>
#include <aws/core/Aws.h>

struct ChangeUnverifiedEmailContext : public Aws::Client::AsyncCallerContext
{
	IDType eID;
	BYTE* hashedEmailToken;

	~ChangeUnverifiedEmailContext()
	{
		if (hashedEmailToken != nullptr) {
			delete[] hashedEmailToken;
			hashedEmailToken = nullptr;
		}
	}
};

class EmailManager : public PKeyOwner
{
public:

	static const std::string EMAIL_CONFIRM_URL;
	static const std::string HTML_DIR;
	static const std::string PWD_RESET_URL;

	void ChangeUnverifiedEmailHandler(const Aws::SES::SESClient* client,
		const Aws::SES::Model::SendEmailRequest& request,
		const Aws::SES::Model::SendEmailOutcome& outcome,
		const AwsSharedPtr<const Aws::Client::AsyncCallerContext>& context);

	void ChangeEmailNotificationHandler(const Aws::SES::SESClient* client,
		const Aws::SES::Model::SendEmailRequest& request,
		const Aws::SES::Model::SendEmailOutcome& outcome);

	EmailManager(BB_Server* bbServer, EmployeeManager* employeeManager);

	bool initSESClient();

	void handleB0(boost::shared_ptr<IPacket> iPack);

	void handleB2(boost::shared_ptr<IPacket> iPack);

	void handleB4(boost::shared_ptr<IPacket> iPack);

	bool setUnverifiedEmail(IDType eID, Aws::String email, std::string& urlEncodedEmailToken, DBManager* dbManager);
	bool setUnverifiedEmail(IDType eID, Aws::String email, BYTE* hashedEmailToken, DBManager* dbManager);

	bool sendEmail(const std::string& sendToAddress, const std::string& senderAddress, const std::string& senderName, const std::string& subject, const std::string& body, Aws::SES::SendEmailResponseReceivedHandler handler, const AwsSharedPtr<const Aws::Client::AsyncCallerContext> context = nullptr, bool isHTML = false);

	IDType emailToEID(const std::string& email, DBManager* dbManager);
	IDType verifiedEmailToEID(const std::string& email, DBManager* dbManager);
	IDType unverifiedEmailToEID(const std::string& email, DBManager* dbManager);

	bool sendVerificationEmail(const std::string& sendToAddress, const std::string& urlEncodedEmailToken, const Aws::SES::SendEmailResponseReceivedHandler& handler, const AwsSharedPtr<const Aws::Client::AsyncCallerContext>& context = nullptr);
	bool sendPwdResetEmail(const std::string& sendToAddress, const std::string& urlEncodedPwdToken, const Aws::SES::SendEmailResponseReceivedHandler& handler, const AwsSharedPtr<const Aws::Client::AsyncCallerContext>& context = nullptr);
	bool sendChangeEmail(const std::string& sendToAddress, const Aws::SES::SendEmailResponseReceivedHandler& handler, const AwsSharedPtr<const Aws::Client::AsyncCallerContext>& context = nullptr);

	bool getVerifiedEmail(IDType eID, std::string& email, DBManager* dbManager);
	bool getUnverifiedEmail(IDType eID, std::string& email, DBManager* dbManager);
	~EmailManager();

private:
	bool verifyEmail(IDType eID, DBManager* dbManager);
	bool removeUnverifiedEmail(IDType eID, DBManager* dbManager);
	bool getEmailToken(IDType eID, BYTE* dbEmailTokenHash, OTL_BIGINT& tokenTime, DBManager* dbManager);
	BB_Server* bbServer;

	EmployeeManager* employeeManager;
	AwsSharedPtr<Aws::SES::SESClient> sesClient;
};
