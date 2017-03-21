#pragma once
#include <stdint.h>
#include <string>
#include <vector>
#include "DBManager.h"

typedef unsigned char BYTE;

class CryptoManager
{
public:
	static void GenerateHash(BYTE* hash, uint32_t hashSize, const BYTE* data, size_t dataSize, const BYTE* salt, uint32_t saltSize);

	static void GenerateHash(BYTE * hash, uint32_t hashSize, const BYTE * data, size_t dataSize);

	static void GenerateRandomData(BYTE* rngData, uint32_t rngDataSize);

	static void UrlEncode(std::string& encoded, const BYTE* data, uint32_t dataSize);

	static void UrlDecode(std::vector <BYTE>& decoded, const std::string& encoded);

	static void OutputBytes(otl_stream& stream, BYTE* data, uint16_t size);

	static void InputBytes(otl_stream& stream, BYTE* data, uint16_t size);
};
