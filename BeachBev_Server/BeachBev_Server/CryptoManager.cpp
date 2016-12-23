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

void CryptoManager::GenerateHash(BYTE * hash, uint32_t hashSize, const BYTE * data, size_t dataSize)
{
		CryptoPP::SHA512 hashFunc;
		if (hashSize >= CryptoPP::SHA512::DIGESTSIZE)
		{
				hashFunc.CalculateDigest(hash, data, dataSize);
		}
		else
		{
				byte digest[CryptoPP::SHA512::DIGESTSIZE];
				hashFunc.CalculateDigest(digest, data, dataSize);
				for (int i = 0; i < hashSize; i++) {
						hash[i] = digest[i];
				}
		}
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

void CryptoManager::UrlDecode(std::vector <BYTE>& decoded, const std::string& encoded)
{
		CryptoPP::Base64Decoder decoder;
		decoder.Put((BYTE*)encoded.data(), encoded.size());
		decoder.MessageEnd();
		CryptoPP::word64 size = decoder.MaxRetrievable();
		if (size) {
				decoded.resize(size);
				decoder.Get((byte*)decoded.data(), decoded.size());
		}
}
