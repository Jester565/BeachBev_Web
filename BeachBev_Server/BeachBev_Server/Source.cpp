#include <iostream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <fstream>
#include "DBManager.h"

static const std::string& CONNECT_INFORMATION_PATH = "./mysql.coni";

int main()
{
		DBManager dbManager;
		
		if (dbManager.connect(ConnectionInformation(CONNECT_INFORMATION_PATH)))
		{
				std::cout << "Connection successful" << std::endl;
				{
						try {
								otl_stream otlStream(50, "INSERT INTO Orders VALUES \
										(:f1<int>, :f2<int>, :f3<char[45]>, :f4<int>, :f5<int>, :f6<double>, \
										:f7<double>);",
										*dbManager.getConnection());
								otlStream << 1;
								otlStream << 0;
								otlStream << "akjdlkfjweaqeladjfa;";
								otlStream << 0;
								otlStream << (int)static_cast<long int>(std::time(nullptr));
								otlStream << 72.2;
								otlStream << 223.1;
						}
						catch (otl_exception ex)
						{
								std::cerr << "OTL EXCEPTION\nCode: " << ex.code << std::endl << "MSG: " << ex.msg << std::endl;
						}
				}
				{
						try {
								otl_stream otlStream(50, "SELECT top 1 * FROM Orders ORDER BY orderID desc;", *dbManager.getConnection());
								dbManager.getConnection()->commit();
								int orderID = 0;
								if (!otlStream.eof())
								{
										otlStream >> orderID;
								}
						}
						catch (otl_exception ex)
						{
								std::cerr << "OTL EXCEPTION\nCode: " << ex.code << std::endl << "MSG: " << ex.msg << std::endl;
						}
				}
		}
		else
		{
				std::cerr << "Connection failed" << std::endl;
		}
		system("pause");
}