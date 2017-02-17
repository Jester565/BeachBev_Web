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
class EmailManager;
typedef uint16_t DeviceID;

static const int TOKEN_SIZE = 64;
static const int HASH_SIZE = 64;
static const int SALT_SIZE = 32;
static const int NAME_SIZE = 50;
static const int EMAIL_SIZE = 254;
static const int MAX_TOKEN_HOURS = 24;

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
	static bool CheckInTimeRange(OTL_BIGINT& time, int numHours);
	
		EmployeeManager(BB_Server* server);

		/// <summary>
		/// Adds a new employee to the database, replies with A1
		/// </summary>
		/// <param name="iPack">The input packet containing data
		/// for protobuf packet A0.</param>
		void handleA0(boost::shared_ptr<IPacket> iPack);

		/// <summary>
		/// Logs in an employee using the pwdToken, replies with A1
		/// </summary>
		/// <param name="iPack">The input packet containing data
		/// for protobuf packet A2.</param>
		void handleA2(boost::shared_ptr<IPacket> iPack);
		
		/// <summary>
		/// Logs in an employee using the username and pwd, replies with A1
		/// </summary>
		/// <param name="iPack">The input packet containing data
		/// for protobuf packet A3.</param>
		void handleA3(boost::shared_ptr<IPacket> iPack);
		
		/// <summary>
		/// Sends a password reset email to email specified by iPack
		/// </summary>
		/// <param name="iPack">The input packet containing data
		/// for protobuf packet A4.</param>
		void handleA4(boost::shared_ptr<IPacket> iPack);
		
		/// <summary>
		/// Checks if the password reset token is valid
		/// </summary>
		/// <param name="iPack">The input packet containing data
		/// for the protobuf packet A6.</param>
		void handleA6(boost::shared_ptr<IPacket> iPack);
		
		/// <summary>
		/// Checks password reset token and resets token if valid
		/// </summary>
		/// <param name="iPack">The input packet containing data
		/// for the protobuf packet A8</param>
		void handleA8(boost::shared_ptr<IPacket> iPack);

		BB_Client* getEmployee(IDType eID);

		std::unordered_map<IDType, Client*> employees;

		~EmployeeManager();

protected:
		IDType addEmployeeToDatabase(const std::string& name, DBManager* dbManager);
		bool setPwd(IDType eID, const std::string& pwd, DBManager* dbManager);
		bool setPwdToken(IDType eID, std::string& urlEncodedPwdToken, DeviceID deviceID, DBManager* dbManager);
		bool setPwdResetToken(IDType eID, std::string& urlEncodedPwdResetToken, DBManager* dbManager);
		bool clearPwdTokens(IDType eID, DBManager* dbManager);
		DeviceID addPwdToken(IDType eID, std::string& urlEncodedPwdToken, DBManager* dbManager);
		DeviceID getNextDeviceID(IDType eID, DBManager* dbManager);
		IDType nameToEID(const std::string & name, DBManager * dbManager);
		IDType getNextEID(DBManager* dbManager);

		bool getPwdData(IDType eID, BYTE* hash, BYTE* salt, DBManager* dbManager);
		bool getPwdToken(IDType eID, BYTE* databaseTokenHash, OTL_BIGINT& tokenTime, DeviceID devID, DBManager* dbManager);
		bool checkPwdResetToken(const std::string& urlEncodedPwdToken, IDType& eID, OTL_BIGINT& tokenTime, DBManager* dbManager);

		bool removePwdResetToken(IDType eID, DBManager* dbManager);
		void loginClient(BB_Client* bbClient, IDType eID);
		EmailManager* emailManager;
		BB_Server* bbServer;
};
