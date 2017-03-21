#pragma once
#include <string>

class DebugManager
{
public:
	static bool DEBUG_ENABLED;
	static bool WARN_ENABLED;

	static void PrintDebug(const std::string& debugMsg);

	static void PrintDebug(char* debugMsg);

	static void PrintWarn(const std::string& debugMsg);

	static void PrintWarn(char* debugMsg);
};
