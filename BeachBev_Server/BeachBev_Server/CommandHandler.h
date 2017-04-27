#pragma once
#include "stdafx.h"
#include <unordered_map>
#include <functional>
#include <string>

class BB_Server;

typedef std::function<bool(BB_Server*)> CommandFunc;
class CommandHandler
{
public:
	CommandHandler(BB_Server* bbServer);

	void createCommands();

	void run();

	bool StopInputCommand(BB_Server*);

	~CommandHandler();

private:
	std::unordered_map <std::string, CommandFunc> commands;
	BB_Server* bbServer;
	bool readInput;
};
