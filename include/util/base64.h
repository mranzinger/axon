/*
 * File description: base64.h
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#ifndef BASE64_H_
#define BASE64_H_

#include <vector>
#include <string>

namespace axon { namespace util {

class CBase64
{
public:
	typedef unsigned char byte;

	static const std::string CHARS;

	static bool IsBase64(byte c);

	static std::string Encode(const byte *a_buf, size_t a_bufLen);
	static std::string Encode(const char *a_buf, size_t a_bufLen)
	{
		return Encode((const byte *)a_buf, a_bufLen);
	}

	static void Encode(const byte *a_buf, size_t a_bufLen, std::string &a_opBuf);
	static void Encode(const char *a_buf, size_t a_bufLen, std::string &a_opBuf)
	{
		Encode((const byte *)a_buf, a_bufLen, a_opBuf);
	}

	static std::vector<byte> Decode(const std::string &a_buf);

	static size_t Decode(const std::string &a_inputBuf, byte *a_outputBuf, size_t a_outputBufSize);

	static size_t Decode(const char *a_inputBuf, size_t a_inputBufSize, byte *a_outputBuf, size_t a_outputBufSize);

private:
	template<typename ExpandFn>
	static size_t Decode(const byte *a_inputBuf, size_t a_inputBufSize, byte *a_outputBuf, byte *a_endOutputBuf, ExpandFn a_fn);
};

}
}



#endif /* BASE64_H_ */
