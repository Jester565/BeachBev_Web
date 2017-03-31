#pragma once
#include "stdafx.h"
#include "BB_Client.h"
#include <PKeyOwner.h>
#include <aws/sts/STSClient.h>
#include <aws/s3/S3Client.h>
#include <string>

class BB_Server;
class BB_Client;
class EmailManager;
class MasterManager;

struct RequestResumePermContext : public Aws::Client::AsyncCallerContext
{
	int clientID;
	std::string folderObjKey;
};

struct HasResumeContext : public Aws::Client::AsyncCallerContext
{
	int clientID;
};

class ResumeManager : public PKeyOwner
{
public:
	static const int USER_RESUME_DURATION = 900;
	static const std::string USER_RESUME_POLICY_PT1;
	static const std::string USER_RESUME_POLICY_PT2;
	static const std::string USER_RESUME_POLICY_PT3;
	static const std::string USER_RESUME_POLICY_PT4;

	static const std::string MASTER_RESUME_POLICY_PT1;
	static const std::string MASTER_RESUME_POLICY_PT2;
	static const std::string MASTER_RESUME_POLICY_PT3;
	static const std::string IAM_USER_NAME;

	static const std::string RESUME_BUCKET_NAME;

	static const std::string RESUME_BUCKET_ARN;

	ResumeManager(BB_Server* bbServer, EmailManager* emailManager, MasterManager* masterManager);

	bool initStsClient();

	bool initS3Client();

	void handleD0(boost::shared_ptr<IPacket> iPack);

	void handleD2(boost::shared_ptr<IPacket> iPack);

	void handleD3(boost::shared_ptr<IPacket> iPack);

	~ResumeManager();

private:
	bool requestResumePermissions(BB_Client* sender, const std::string& policy);

	void requestResumeHandler(const Aws::STS::STSClient* stsClient,
		const Aws::STS::Model::GetFederationTokenRequest& req,
		const Aws::STS::Model::GetFederationTokenOutcome& outcome,
		const AwsSharedPtr<const Aws::Client::AsyncCallerContext>& context);

	void createUserResumePolicy(IDType eID, std::string& policy);

	void createMasterResumePolicy(std::string& policy);

	void hasResumeHandler(const Aws::S3::S3Client* s3Client,
		const Aws::S3::Model::ListObjectsV2Request& req,
		const Aws::S3::Model::ListObjectsV2Outcome& outcome,
		const AwsSharedPtr<const Aws::Client::AsyncCallerContext>& context);

	BB_Server* bbServer;
	MasterManager* masterManager;
	EmailManager* emailManager;
	AwsSharedPtr<Aws::STS::STSClient> stsClient;
	AwsSharedPtr<Aws::S3::S3Client> s3Client;
};
