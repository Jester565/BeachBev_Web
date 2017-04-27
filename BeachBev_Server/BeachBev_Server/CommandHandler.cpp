#include "CommandHandler.h"
#include <Logger.h>
#include <iostream>
#include <thread>

bool ExitCommand(BB_Server* bbServer) {
	std::cout << "Exiting..." << std::endl;
	return false;
}

bool SetLogCommand(BB_Server* bbServer) {
	std::cout << "Select log level:\n\
0) DebugLow\n\
1) DebugHigh\n\
2) Warn\n\
3) Error\n\
Selection: ";
	int logLevel = 0;
	std::cin >> logLevel;
	Logger::SetLogLevel((LOG_LEVEL)logLevel);
}

bool CommandHandler::StopInputCommand(BB_Server* bbServer) {
	std::cout << "No longer accepting input" << std::endl;
	readInput = false;
	return true;
}

CommandHandler::CommandHandler(BB_Server* bbServer)
	:bbServer(bbServer), readInput(true)
{
	createCommands();
}

void CommandHandler::createCommands()
{
	commands.emplace(std::make_pair("stopInput", (CommandFunc)std::bind(&CommandHandler::StopInputCommand, this, std::placeholders::_1)));
	commands.emplace(std::make_pair("exit", CommandFunc(&ExitCommand)));
	commands.emplace(std::make_pair("setLog", CommandFunc(&SetLogCommand)));
}

void CommandHandler::run()
{
	std::cout << "Type \'exit\' to end the task cleanly" << std::endl;
	while (true) {
		std::string cmd;
		if (readInput) {
			std::cin >> cmd;
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::seconds(20));
		}
		auto mapIter = commands.find(cmd);
		if (mapIter != commands.end()) {
			if (!mapIter->second(bbServer))
			{
				return;
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