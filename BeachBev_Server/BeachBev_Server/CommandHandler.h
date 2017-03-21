#pragma once
#include <unordered_map>
#include <functional>
#include <string>

class BB_Server;

class CommandHandler
{
public:
	CommandHandler(BB_Server* bbServer);

	void createCommands();

	void run();

	~CommandHandler();

private:
	std::unordered_map <std::string, std::function<bool(BB_Server*)>> commands;
	BB_Server* bbServer;
};
