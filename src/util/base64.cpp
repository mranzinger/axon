/*
 * File description: base64.cpp
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */
#include "base64.h"
#include <iostream>

namespace axon { namespace util {

const std::string CBase64::CHARS =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

bool CBase64::IsBase64(byte c)
{
	return isalnum(c) || '+' == c || '/' == c;
}

std::string CBase64::Encode(const byte* a_buf, size_t a_bufLen)
{
	std::string l_ret;

	Encode(a_buf, a_bufLen, l_ret);

	return l_ret;
}

void CBase64::Encode(const byte* a_buf, size_t a_bufLen, std::string& a_opBuf)
{
	int i = 0;
	int j = 0;
	byte l_char_array_3[3];
	byte l_char_array_4[4];

	while (a_bufLen--) {
		l_char_array_3[i++] = *(a_buf++);
		if (i == 3) {
			l_char_array_4[0] = (l_char_array_3[0] & 0xfc) >> 2;
			l_char_array_4[1] = ((l_char_array_3[0] & 0x03) << 4) + ((l_char_array_3[1] & 0xf0) >> 4);
			l_char_array_4[2] = ((l_char_array_3[1] & 0x0f) << 2) + ((l_char_array_3[2] & 0xc0) >> 6);
			l_char_array_4[3] = l_char_array_3[2] & 0x3f;

			for(i = 0; (i <4) ; i++)
				a_opBuf += CHARS[l_char_array_4[i]];
			i = 0;
		}
	}

	if (i)
	{
		for(j = i; j < 3; j++)
			l_char_array_3[j] = '\0';

		l_char_array_4[0] = (l_char_array_3[0] & 0xfc) >> 2;
		l_char_array_4[1] = ((l_char_array_3[0] & 0x03) << 4) + ((l_char_array_3[1] & 0xf0) >> 4);
		l_char_array_4[2] = ((l_char_array_3[1] & 0x0f) << 2) + ((l_char_array_3[2] & 0xc0) >> 6);
		l_char_array_4[3] = l_char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
			a_opBuf += CHARS[l_char_array_4[j]];

		while((i++ < 3))
			a_opBuf += '=';
	}
}

std::vector<CBase64::byte> CBase64::Decode(const std::string& a_buf)
{
	std::vector<byte> l_ret(1000);

	size_t l_size = Decode((const byte *)a_buf.c_str(), a_buf.size(), l_ret.data(), l_ret.data() + l_ret.size(),
			[&l_ret] (byte *&a_begin, byte *&a_end, byte *&a_curr)
			{
				l_ret.resize(l_ret.size() * 2);

				a_curr = l_ret.data() + (a_curr - a_begin);
				a_begin = l_ret.data();
				a_end = l_ret.data() + l_ret.size();

				return true;
			});

	l_ret.resize(l_size);
	return l_ret;
}



size_t CBase64::Decode(const std::string& a_inputBuf, byte* a_outputBuf, size_t a_outputBufSize)
{
	return Decode(a_inputBuf.c_str(), a_inputBuf.size(), a_outputBuf, a_outputBufSize);
}

size_t CBase64::Decode(const char* a_inputBuf, size_t a_inputBufSize, byte* a_outputBuf, size_t a_outputBufSize)
{
	return Decode((const byte *)a_inputBuf, a_inputBufSize, a_outputBuf, a_outputBuf + a_outputBufSize,
			[] (byte *&a_begin, byte *&a_end, byte *&a_curr)
			{
				return false;
			});
}

template<typename ExpandFn>
size_t CBase64::Decode(const byte *a_inputBuf, size_t a_inputBufSize, byte* a_outputBuf, byte* a_endOutputBuf, ExpandFn a_fn)
{
	if (a_endOutputBuf == a_outputBuf)
	{
		if (!a_fn(a_outputBuf, a_endOutputBuf, a_outputBuf))
			return SIZE_MAX;
	}

	size_t in_len = a_inputBufSize;
	int i = 0;
	int j = 0;
	int in_ = 0;
	byte char_array_4[4], char_array_3[3];
	byte *l_opCurr = a_outputBuf;

	auto l_pushBack = [&] (byte a_val) -> bool
		{
			if (l_opCurr == a_endOutputBuf)
			{
				if (!a_fn(a_outputBuf, a_endOutputBuf, l_opCurr))
					return false;
			}

			*l_opCurr++ = a_val;
			return true;
		};

	while (in_len-- && ( a_inputBuf[in_] != '=') && IsBase64(a_inputBuf[in_])) {
		char_array_4[i++] = a_inputBuf[in_]; in_++;
		if (i ==4) {
			for (i = 0; i <4; i++)
				char_array_4[i] = CHARS.find(char_array_4[i]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++)
			{
				if (!l_pushBack(char_array_3[i]))
					return SIZE_MAX;
			}
			i = 0;
		}
	}

	if (i) {
		for (j = i; j <4; j++)
			char_array_4[j] = 0;

		for (j = 0; j <4; j++)
			char_array_4[j] = CHARS.find(char_array_4[j]);

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++)
		{
			if (!l_pushBack(char_array_3[j]))
				return SIZE_MAX;
		}
	}

	return l_opCurr - a_outputBuf;
}



} }
