#include "HiCommand.h"
#include <iostream>

HiCommand::HiCommand()
		:Command("hi")
{
}

void HiCommand::run(BB_Server * bbServer)
{
		std::cout << "HI to you too" << std::endl;
}

HiCommand::~HiCommand()
{
}
