#include "DebugManager.h"
#include <iostream>

bool DebugManager::DEBUG_ENABLED = true;
bool DebugManager::WARN_ENABLED = true;

void DebugManager::PrintDebug(const std::string & debugMsg)
{
		if (DEBUG_ENABLED)
				std::cout << debugMsg << std::endl;
}

void DebugManager::PrintDebug(char * debugMsg)
{
		if (DEBUG_ENABLED)
				std::cout << debugMsg << std::endl;
}

void DebugManager::PrintWarn(const std::string & debugMsg)
{
		if (WARN_ENABLED)
				std::cout << debugMsg << std::endl;
}

void DebugManager::PrintWarn(char * debugMsg)
{
		if (WARN_ENABLED)
				std::cout << debugMsg << std::endl;
}
