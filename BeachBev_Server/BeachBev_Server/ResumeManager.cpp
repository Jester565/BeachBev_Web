#include "ResumeManager.h"
#include "Packets/BBPacks.pb.h"
#include "BB_Server.h"
#include "BB_Client.h"
#include "EmailManager.h"
#include <ClientManager.h>
#include <WSOPacket.h>
#include <WSIPacket.h>
#include <boost/make_shared.hpp>
#include <aws/sts/model/GetFederationTokenRequest.h>

const std::string ResumeManager::USER_RESUME_BUCKET_ARN = "aws:s3:::beachbev-resumes";

const std::string ResumeManager::USER_RESUME_POLICY_PT1 = "{\
\"Version\": \"2012-10-17\",\
\"Statement\" : [\
{\
{\
	\"Sid\": \"AllowListingOfUserFolder\",\
  \"Effect\": \"Allow\",\
  \"Action\": [\
      \"s3:ListBucket\"\
  ],\
  \"Resource\": [\
      \"arn:";
const std::string ResumeManager::USER_RESUME_POLICY_PT2 = "\"\
  ],\
	\"Condition\":{\"StringLike\":{\"s3:prefix\":[\"";

const std::string ResumeManager::USER_RESUME_POLICY_PT3 = "/*\"]}}\
},\
	\"Sid\": \"Stmt1487575487000\",\
		\"Effect\" : \"Allow\",\
		\"Action\" : [\
			\"s3:GetObject\",\
				\"s3:PutObject\"\
		],\
		\"Resource\" : [\
			\"arn:";

const std::string ResumeManager::USER_RESUME_POLICY_PT4 = "/*\"\
		],\
}\
]\
}";

const std::string ResumeManager::USER_RESUME_NAME = "pdf_usr";

ResumeManager::ResumeManager(BB_Server* bbServer, EmailManager* emailManager)
	:PKeyOwner(bbServer->getPacketManager()), bbServer(bbServer), emailManager(emailManager)
{
	if (!initStsClient()) {
		std::cerr << "Could not initialize STS client!" << std::endl;
	}
	addKey(new PKey("D0", this, &ResumeManager::handleD0));
}

bool ResumeManager::initStsClient()
{
	Aws::Client::ClientConfiguration clientConfig;
	clientConfig.region = AWS_SERVER_REGION_1;
	stsClient = Aws::MakeShared<Aws::STS::STSClient>(AWS_ALLOC_TAG, clientConfig);
}

void ResumeManager::handleD0(boost::shared_ptr<IPacket> iPack)
{
	BB_Client* sender = (BB_Client*)bbServer->getClientManager()->getClient(iPack->getSentFromID());
	std::string email;
	if (sender->getEmpID() > 0) {
		if (emailManager->getVerifiedEmail(sender->getEmpID(), email, sender->getDBManager())) {
			requestUserResumePermissions(sender);
		}
		else
		{
			ProtobufPackets::PackD1 replyPacket;
			replyPacket.set_msg("Email is not verified");
			boost::shared_ptr<OPacket> oPack = boost::make_shared<WSOPacket>("D1");
			oPack->setSenderID(0);
			oPack->addSendToID(sender->getID());
			oPack->setData(boost::make_shared<std::string>(replyPacket.SerializeAsString()));
			bbServer->getClientManager()->send(oPack, sender);
		}
	}
}

ResumeManager::~ResumeManager()
{
}

bool ResumeManager::requestUserResumePermissions(BB_Client * sender)
{
	auto context = Aws::MakeShared<RequestResumePermContext>(AWS_ALLOC_TAG);
	context->clientID = sender->getID();
	context->folderObjKey = std::to_string(sender->getEmpID());

	std::string policy = USER_RESUME_POLICY_PT1;
	policy += USER_RESUME_BUCKET_ARN;
	policy += USER_RESUME_POLICY_PT2;
	policy += USER_RESUME_BUCKET_ARN;
	policy += "/";
	policy += context->folderObjKey;
	policy += USER_RESUME_POLICY_PT3;
	policy += USER_RESUME_BUCKET_ARN;
	policy += "/";
	policy += context->folderObjKey;
	policy += USER_RESUME_POLICY_PT4;
	Aws::STS::Model::GetFederationTokenRequest request;
	request.SetPolicy(policy.c_str());
	request.SetDurationSeconds(USER_RESUME_DURATION);
	request.SetName(USER_RESUME_NAME.c_str());
	stsClient->GetFederationTokenAsync(request, std::bind(&ResumeManager::requestResumeHandler, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
}

void ResumeManager::requestResumeHandler(const Aws::STS::STSClient * stsClient, const Aws::STS::Model::GetFederationTokenRequest & req, const Aws::STS::Model::GetFederationTokenOutcome & outcome, const AwsSharedPtr<const Aws::Client::AsyncCallerContext>& context)
{
	auto resumeContext = std::static_pointer_cast<const RequestResumePermContext>(context);
	BB_Client* sender = (BB_Client*)bbServer->getClientManager()->getClient(resumeContext->clientID);
	if (sender != nullptr) {
		ProtobufPackets::PackD1 replyPacket;
		if (outcome.IsSuccess()) {
			auto credentials = outcome.GetResult().GetCredentials();
			replyPacket.set_folderobjkey(resumeContext->folderObjKey);
			replyPacket.set_accesskeyid(AwsStrToStr(credentials.GetAccessKeyId()));
			replyPacket.set_accesskey(AwsStrToStr(credentials.GetSecretAccessKey()));
			replyPacket.set_sessionkey(AwsStrToStr(credentials.GetSessionToken()));
		}
		else
		{
			replyPacket.set_msg("Failed to request access id: " + AwsStrToStr(outcome.GetError().GetMessage()));
			std::cerr << replyPacket.msg();
		}
		boost::shared_ptr<OPacket> oPack = boost::make_shared<WSOPacket>("D1");
		oPack->setSenderID(0);
		oPack->addSendToID(sender->getID());
		oPack->setData(boost::make_shared<std::string>(replyPacket.SerializeAsString()));
		bbServer->getClientManager()->send(oPack, sender);
	}
}
