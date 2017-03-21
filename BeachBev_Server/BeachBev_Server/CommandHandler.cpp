#include "CommandHandler.h"
#include <iostream>

bool HiCommand(BB_Server* bbServer) {
	std::cout << "Hello to you too" << std::endl;
	return true;
}

bool ExitCommand(BB_Server* bbServer) {
	std::cout << "Exiting..." << std::endl;
	return false;
}

CommandHandler::CommandHandler(BB_Server* bbServer)
	:bbServer(bbServer)
{
	createCommands();
}

void CommandHandler::createCommands()
{
	commands.emplace(std::make_pair("hi", std::function<bool(BB_Server*)>(&HiCommand)));
	commands.emplace(std::make_pair("exit", std::function<bool(BB_Server*)>(&ExitCommand)));
}

void CommandHandler::run()
{
	std::cout << "Type \'exit\' to end the task cleanly" << std::endl;
	while (true) {
		std::string cmd;
		std::cin >> cmd;
		auto mapIter = commands.find(cmd);
		if (mapIter != commands.end()) {
			if (!mapIter->second(bbServer))
			{
				break;
			}
		}
		else
		{
			std::cout << "Command not found" << std::endl;
		}
	}
}

CommandHandler::~CommandHandler()
{
}