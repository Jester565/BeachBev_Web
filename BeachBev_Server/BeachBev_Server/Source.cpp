#include <iostream>
#include "DBManager.h"

int main()
{
		std::cout << "Attemping to connect..." << std::endl;
		DBManager dbManager;
		std::cout << "Enter username: ";
		std::string userName;
		std::cin >> userName;
		std::cout << "Enter password: ";
		std::string pwd;
		std::cin >> pwd;
		if (dbManager.connect("LAPTOP-6SS7RV3H\\SQLEXPRESS", pwd, userName, "beach_bev", "SQL Server"))
		{
				std::cout << "Connection successful" << std::endl;
		}
		else
		{
				std::cerr << "Connection failed" << std::endl;
		}
		system("pause");
}