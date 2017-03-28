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

const std::string ResumeManager::USER_RESUME_POLICY_PT1 = "{\
\"Version\": \"2012-10-17\",\
\"Statement\" : [\
{\
	\"Sid\": \"Stmt1487575487000\",\
		\"Effect\" : \"Allow\",\
		\"Action\" : [\
			\"s3:GetObject\",\
				\"s3:PutObject\"\
		],\
		\"Resource\" : [\
			\"arn:";

const std::string ResumeManager::USER_RESUME_BUCKET_ARN = "aws:s3:::beachbev-resumes/";

const std::string ResumeManager::USER_RESUME_POLICY_PT2 = "/*\"\
		]\
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
	std::string usrArn = USER_RESUME_BUCKET_ARN;
	usrArn += std::to_string(sender->getEmpID());

	auto context = Aws::MakeShared<RequestResumePermContext>(AWS_ALLOC_TAG);
	context->clientID = sender->getID();
	context->arn = usrArn;

	std::string policy = USER_RESUME_POLICY_PT1;
	policy += USER_RESUME_BUCKET_ARN;
	policy += usrArn;
	policy += USER_RESUME_POLICY_PT2;
	Aws::STS::Model::GetFederationTokenRequest request;
	request.SetPolicy(policy);
	request.SetDurationSeconds(USER_RESUME_DURATION);
	request.SetName(USER_RESUME_NAME);
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
			replyPacket.set_allocated_folderobjkey = resumeContext->arn;
			replyPacket.set_accesskeyid(credentials.GetAccessKeyId());
			replyPacket.set_accesskey(credentials.GetSecretAccessKey());
			replyPacket.set_sessionkey(credentials.GetSessionToken());
		}
		else
		{
			replyPacket.set_msg("Failed to request access id: " + std::string(outcome.GetError().GetMessage()));
			std::cerr << replyPacket.msg();
		}
		boost::shared_ptr<OPacket> oPack = boost::make_shared<WSOPacket>("D1");
		oPack->setSenderID(0);
		oPack->addSendToID(sender->getID());
		oPack->setData(boost::make_shared<std::string>(replyPacket.SerializeAsString()));
		bbServer->getClientManager()->send(oPack, sender);
	}
}