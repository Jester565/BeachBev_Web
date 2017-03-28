#pragma once
#include "stdafx.h"
#include <PKeyOwner.h>
#include <aws/sts/STSClient.h>
#include <string>

class BB_Server;
class BB_Client;
class EmailManager;

struct RequestResumePermContext
{
	int clientID;
	std::string arn;
};

class ResumeManager : public PKeyOwner
{
public:
	static const int USER_RESUME_DURATION = 900;
	static const std::string USER_RESUME_POLICY_PT1;
	static const std::string USER_RESUME_POLICY_PT2;
	static const std::string USER_RESUME_NAME;

	static const std::string USER_RESUME_BUCKET_ARN;

	ResumeManager(BB_Server* bbServer, EmailManager* emailManager);

	bool initStsClient();

	void handleD0(boost::shared_ptr<IPacket> iPack);

	~ResumeManager();

private:
	bool requestUserResumePermissions(BB_Client* sender);

	void requestResumeHandler(const Aws::STS::STSClient* stsClient,
		const Aws::STS::Model::GetFederationTokenRequest& req,
		const Aws::STS::Model::GetFederationTokenOutcome& outcome,
		const AwsSharedPtr<const Aws::Client::AsyncCallerContext>& context);

	BB_Server* bbServer;
	EmailManager* emailManager;
	AwsSharedPtr<Aws::STS::STSClient> stsClient;
};
