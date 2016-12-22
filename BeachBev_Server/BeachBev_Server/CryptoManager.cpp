#include "CryptoManager.h"
#include "DBManager.h"
#include <cryptopp/pwdbased.h>
#include <cryptopp/osrng.h>
#include <cryptopp/secblock.h>
#include <cryptopp/base64.h>


void CryptoManager::GenerateHash(BYTE * hash, uint32_t hashSize, const BYTE * data, size_t dataSize, const BYTE * salt, uint32_t saltSize)
{
		CryptoPP::PKCS5_PBKDF2_HMAC <CryptoPP::SHA256> pbkdf2;
		pbkdf2.DeriveKey(hash, hashSize, 0, data, dataSize, salt, saltSize, 80020);
}

void CryptoManager::GenerateRandomData(BYTE * rngData, uint32_t rngDataSize)
{
		CryptoPP::AutoSeededRandomPool rng;
		rng.GenerateBlock(rngData, rngDataSize);
}

void CryptoManager::UrlEncode(std::string & encoded, const BYTE * data, uint32_t dataSize)
{
		CryptoPP::Base64URLEncoder encoder;
		encoder.Put(data, dataSize);
		encoder.MessageEnd();
		CryptoPP::word64 size = encoder.MaxRetrievable();
		if (size) {
				encoded.resize(size);
				encoder.Get((byte*)encoded.data(), encoded.size());
		}
}
