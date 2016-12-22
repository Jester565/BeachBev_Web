#pragma once
#include <vector>
class EmailManager
{
public:
		static const int EMAIL_TOKEN_SIZE = 64;
		static const int EMAIL_HASH_SIZE = 128;
		static const std::string EMAIL_CONFIRM_URL;
		static const std::vector <BYTE> EMAIL_SALT;

		EmailManager();

		bool sendEmailVerification(IDType eID, DBManager* dbManager, const std::string& emailAddress);

		bool setEmailTokenHash(IDType eID, DBManager* dbManager, BYTE* emailTokenHash);

		bool sendEmail(const std::string& sendToAddress, const std::string& senderAddress, const std::string& senderName, const std::string& subject, const std::string& body, bool isHTML = false);

		~EmailManager();
};
