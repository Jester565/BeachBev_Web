#pragma once
#include <Macros.h>
#include <PKeyOwner.h>
#include <WSIPacket.h>
#include <unordered_map>
#include <base64_converter.h>

class DBManager;
class BB_Server;
class BB_Client;
class Client;

template <typename T>
void CmdInjectProtect(T data, uint32_t dataSize) {
		for (int i = 0; i < dataSize; i++)
		{
				if (data[i] == '\'' || data[i] == '\"')
				{
						data[i] = 33;
				}
				else if (data[i] == '\0')
				{
						data[i] = 37;
				}
		}
}

template <typename T>
bool CmdInjectSafe(T data, uint32_t dataSize) {
		for (int i = 0; i < dataSize; i++)
		{
				if (data[i] == '\'' || data[i] == '\"')
				{
						return false;
				}
		}
		return true;
}

class EmployeeManager : public PKeyOwner
{
public:
		static const int TOKEN_SIZE = 64;
		static const int HASH_SIZE = 64;
		static const int SALT_SIZE = 32;
		static const int NAME_SIZE = 50;
		static const int EMAIL_SIZE = 50;
		static const int INITIAL_A_STATE = 0;

		EmployeeManager(BB_Server* server);

		void keyD0(boost::shared_ptr<IPacket> iPack);

		void keyE0(boost::shared_ptr<IPacket> iPack);

		bool addEmployeeToDatabase(IDType eID, DBManager* dbManager, const std::string& name, const std::string& pwd, const std::string& email);

		bool checkPwd(IDType eID, const std::string& pwd, DBManager* dbManager);

		bool setToken(IDType eID, DBManager* dbManager, BYTE* token);

		bool getPwdData(IDType eID, DBManager* dbManager, BYTE* hash, BYTE* salt);

		BB_Client* getEmployee(IDType eID);

		std::unordered_map<IDType, Client*> employees;

		~EmployeeManager();

protected:
		BB_Server* bbServer;
		IDType nameToEID(const std::string & name, DBManager * dbManager);
		IDType getNextEID(DBManager* dbManager);
};
