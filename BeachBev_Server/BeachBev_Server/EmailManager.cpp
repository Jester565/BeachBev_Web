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
#include <aws/core/Aws.h>
#include <aws/email/model/SendEmailRequest.h>
#include <aws/email/SESClient.h>

//TODO
const std::string EmailManager::PWD_RESET_URL = "https://beachbevs.com/pwdReset.html";
const std::string EmailManager::EMAIL_CONFIRM_URL = "https://beachbevs.com/email_confirm.html";
const std::string EmailManager::HTML_DIR = "/home/ubuntu/BeachBev_Web/";

void EmailManager::ChangeUnverifiedEmailHandler(const Aws::SES::SESClient * client, const Aws::SES::Model::SendEmailRequest & request, const Aws::SES::Model::SendEmailOutcome & outcome, const AwsSharedPtr<const Aws::Client::AsyncCallerContext>& context)
{
	auto unverifiedEmailContext = std::static_pointer_cast<const ChangeUnverifiedEmailContext>(context);
	auto bbClient = employeeManager->getEmployee(unverifiedEmailContext->eID);
	if (bbClient != nullptr) {
		ProtobufPackets::PackB1 packB1;
		packB1.set_success(false);
		if (outcome.IsSuccess()) {
			if (setUnverifiedEmail(unverifiedEmailContext->eID, request.GetDestination().GetToAddresses().front(), unverifiedEmailContext->hashedEmailToken, bbClient->getDBManager()))
			{
				packB1.set_success(true);
				packB1.set_msg("Email successfully changed");
			}
			else
			{
				packB1.set_msg("Failed to set unverified email after successful email send");
			}
		}
		else
		{
			packB1.set_msg("Failed to send verification email: " + std::string(outcome.GetError().GetMessage().c_str()));
			std::cerr << "ChangeUnverifiedEmailHandler: " << outcome.GetError().GetMessage().c_str() << std::endl;
		}
		boost::shared_ptr<OPacket> oPack = boost::make_shared<WSOPacket>("B1");
		oPack->setSenderID(0);
		oPack->setData(boost::make_shared<std::string>(packB1.SerializeAsString()));
		bbServer->getClientManager()->send(oPack, bbClient);
	}
}

void EmailManager::ChangeEmailNotificationHandler(const Aws::SES::SESClient * client, const Aws::SES::Model::SendEmailRequest & request, const Aws::SES::Model::SendEmailOutcome & outcome)
{
	if (!outcome.IsSuccess()) {
		std::cerr << "ChanageEmailNotificationHandler: " << outcome.GetError().GetMessage().c_str() << std::endl;
	}
}

EmailManager::EmailManager(BB_Server* bbServer, EmployeeManager* employeeManager)
	:PKeyOwner(bbServer->getPacketManager()), bbServer(bbServer)
{
	addKey(new PKey("B0", this, &EmailManager::handleB0));
	addKey(new PKey("B2", this, &EmailManager::handleB2));
	addKey(new PKey("B4", this, &EmailManager::handleB4));
	initSESClient();
}

bool EmailManager::initSESClient()
{
	Aws::Client::ClientConfiguration clientConfig;
	clientConfig.region = AWS_SERVER_REGION;
	sesClient = Aws::MakeShared<Aws::SES::SESClient>(AWS_ALLOC_TAG, clientConfig);
	return true;
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
				BYTE genToken[TOKEN_SIZE];
				CryptoManager::GenerateRandomData(genToken, TOKEN_SIZE);
				BYTE* genTokenHash = new BYTE[TOKEN_SIZE];
				CryptoManager::GenerateHash(genTokenHash, TOKEN_SIZE, genToken, TOKEN_SIZE);
				std::string urlEncodedEmailToken;
				CryptoManager::UrlEncode(urlEncodedEmailToken, genTokenHash, TOKEN_SIZE);

				AwsSharedPtr<ChangeUnverifiedEmailContext> changeUnverifiedContext = std::make_shared<ChangeUnverifiedEmailContext>();
				changeUnverifiedContext->eID = sender->getEmpID();
				changeUnverifiedContext->hashedEmailToken = genTokenHash;

				sendVerificationEmail(packB0.email(), urlEncodedEmailToken,
					std::bind(&EmailManager::ChangeUnverifiedEmailHandler, this, std::placeholders::_1,
						std::placeholders::_2, std::placeholders::_3, std::placeholders::_4), changeUnverifiedContext);
				replyPacket.set_success(true);
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
	if (!replyPacket.success()) {
		boost::shared_ptr<OPacket> oPack = boost::make_shared<WSOPacket>("B1");
		oPack->setSenderID(0);
		oPack->setData(boost::make_shared<std::string>(replyPacket.SerializeAsString()));
		bbServer->getClientManager()->send(oPack, sender);
	}
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
						sendChangeEmail(prevEmail,
							std::bind(&EmailManager::ChangeEmailNotificationHandler, this, std::placeholders::_1,
								std::placeholders::_2, std::placeholders::_3));
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
			replyPacket.set_msg("The token for this email is already verified, \
you may be <a href=\'javascript:document.location.href=\"login.html?\" + document.location.href\'>logged into the wrong account.</a>");
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

bool EmailManager::setUnverifiedEmail(IDType eID, Aws::String email, std::string& urlEncodedEmailToken, DBManager * dbManager)
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
		otlStream << std::string(email.c_str());
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

bool EmailManager::setUnverifiedEmail(IDType eID, Aws::String email, BYTE* hashedEmailToken, DBManager * dbManager)
{
	std::string query = "REPLACE INTO UnverifiedEmails VALUES (:f1<int>, :f2<char[";
	query += std::to_string(EMAIL_SIZE);
	query += "]>, :f3<raw[";
	query += std::to_string(TOKEN_SIZE);
	query += "]>, :f4<bigint>)";
	try {
		otl_stream otlStream(OTL_BUFFER_SIZE, query.c_str(), *dbManager->getConnection());
		otlStream << (int)eID;
		otlStream << std::string(email.c_str());
		CryptoManager::OutputBytes(otlStream, hashedEmailToken, TOKEN_SIZE);
		otlStream << (OTL_BIGINT)(std::time(NULL));
	}
	catch (otl_exception ex)
	{
		std::cerr << "Code: " << ex.code << std::endl << "MSG: " << ex.msg << std::endl;
	}
	return true;
}

bool EmailManager::sendEmail(const std::string & sendToAddress, const std::string & senderAddress, const std::string& senderName, const std::string & subject, const std::string & body, Aws::SES::SendEmailResponseReceivedHandler handler, const AwsSharedPtr<const Aws::Client::AsyncCallerContext> context, bool isHTML)
{
	Aws::SES::Model::Content sesSubject;
	sesSubject.SetData(subject.c_str());
	Aws::SES::Model::Content sesBodyContent;
	sesBodyContent.SetData(body.c_str());
	Aws::SES::Model::Body sesBody;
	if (isHTML) {
		sesBody.SetHtml(sesBodyContent);
	}
	else
	{
		sesBody.SetText(sesBodyContent);
	}
	Aws::SES::Model::Message msg;
	msg.SetSubject(sesSubject);
	msg.SetBody(sesBody);
	Aws::SES::Model::SendEmailRequest request;
	request.SetSource(senderAddress.c_str());
	Aws::SES::Model::Destination dest;
	dest.AddToAddresses(sendToAddress.c_str());
	request.SetDestination(dest);
	request.SetMessage(msg);
	sesClient->SendEmailAsync(request, handler, context);
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

bool EmailManager::sendVerificationEmail(const std::string& sendToAddress, const std::string& urlEncodedEmailToken, const Aws::SES::SendEmailResponseReceivedHandler& handler, const AwsSharedPtr<const Aws::Client::AsyncCallerContext>& context)
{
	std::string emailURL = EMAIL_CONFIRM_URL + "?" + urlEncodedEmailToken;
	std::stringstream stringIn;
	{
		std::ifstream fileIn(HTML_DIR + "emailPt1.html");
		stringIn << fileIn.rdbuf();
	}
	stringIn << emailURL;
	{
		std::ifstream fileIn(HTML_DIR + "emailPt2.html");
		stringIn << fileIn.rdbuf();
	}
	return sendEmail(sendToAddress, "management@beachbevs.com", "BeachBevs", "Email Verification", stringIn.str(), handler, context, true);
}

bool EmailManager::sendPwdResetEmail(const std::string& sendToAddress, const std::string & urlEncodedPwdToken, const Aws::SES::SendEmailResponseReceivedHandler& handler, const AwsSharedPtr<const Aws::Client::AsyncCallerContext>& context)
{
	std::string pwdResetURL = PWD_RESET_URL + "?" + urlEncodedPwdToken;
	std::stringstream stringIn;
	{
		std::ifstream fileIn(HTML_DIR + "emailPwdResetPt1.html");
		stringIn << fileIn.rdbuf();
	}
	stringIn << pwdResetURL;
	{
		std::ifstream fileIn(HTML_DIR + "emailPwdResetPt2.html");
		stringIn << fileIn.rdbuf();
	}
	return sendEmail(sendToAddress, "management@beachbevs.com", "BeachBevs", "Password Reset", stringIn.str(), handler, context, true);
}

bool EmailManager::sendChangeEmail(const std::string& sendToAddress, const Aws::SES::SendEmailResponseReceivedHandler& handler, const AwsSharedPtr<const Aws::Client::AsyncCallerContext>& context)
{
	std::stringstream stringIn;
	{
		std::ifstream fileIn(HTML_DIR + "emailChange.html");
		stringIn << fileIn.rdbuf();
	}
	return sendEmail(sendToAddress, "management@beachbevs.com", "BeachBevs", "Email Changed", stringIn.str(), handler, context, true);
}