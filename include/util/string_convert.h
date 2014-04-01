/*
 * string_convert.h
 *
 *  Created on: Feb 9, 2014
 *      Author: Mike
 */

#ifndef STRING_CONVERT_H_
#define STRING_CONVERT_H_

#include <sstream>
#include <string>

namespace axon { namespace util {

template<typename T>
struct CToString
{
	static std::string Get(const T &a_val)
	{
		std::ostringstream ss;

		ss << a_val;

		return ss.str();
	}
};

template<typename T>
std::string ToString(const T &val)
{
	return CToString<T>::Get(val);
}

template<typename T>
struct CToWString
{
	static std::wstring Get(const T &a_val)
	{
		std::wostringstream ss;

		ss << a_val;

		return ss.str();
	}
};

template<typename T>
std::wstring ToStringW(const T &val)
{
	return CToWString<T>::Get(val);
}

template<typename T>
struct CStringTo
{
	static T Get(const std::string &s)
	{
		T val;
		Get(s, val);
		return val;
	}

	static void Get(const std::string &s, T &val)
	{
		std::istringstream ss(s);

		ss >> val;

		if (ss.bad())
			throw std::runtime_error("Unable to parse specified string into val");
	}
};

template<typename T>
struct CWStringTo
{
	static T Get(const std::wstring &s)
	{
		T val;
		Get(s, val);
		return val;
	}
	static void Get(const std::wstring &s, T &val)
	{
		std::wistringstream ss(s);

		ss >> val;

		if (ss.bad())
			throw std::runtime_error("Unable to parse specified string into val");
	}
};

template<typename T>
T StringTo(const std::string &s)
{
	return CStringTo<T>::Get(s);
}

template<typename T>
T StringTo(const std::wstring &s)
{
	return CWStringTo<T>::Get(s);
}

template<typename T>
void StringTo(const std::string &s, T &val)
{
	CStringTo<T>::Get(s, val);
}

template<typename T>
void StringTo(const std::wstring &s, T &val)
{
	CWStringTo<T>::Get(s, val);
}

template<>
inline std::string ToString<std::string>(const std::string &s)
{
	return s;
}

template<>
inline std::string StringTo<std::string>(const std::string &s)
{
	return s;
}

template<>
inline void StringTo<std::string>(const std::string &s, std::string &val)
{
	val = s;
}

template<>
inline std::wstring ToStringW<std::wstring>(const std::wstring &s)
{
	return s;
}

template<>
inline std::wstring StringTo<std::wstring>(const std::wstring &s)
{
	return s;
}

template<>
inline void StringTo<std::wstring>(const std::wstring &s, std::wstring &val)
{
	val = s;
}

template<>
inline std::wstring ToStringW<std::string>(const std::string &s)
{
	return std::wstring(s.begin(), s.end());
}

template<>
inline std::string ToString<std::wstring>(const std::wstring &s)
{
	return std::string(s.begin(), s.end());
}

template<>
inline char StringTo<char>(const std::wstring &s)
{
	if (s.empty())
		return 0;
	else
		return (char)s[0];
}

template<>
inline void StringTo<char>(const std::wstring &s, char &c)
{
	if (s.empty())
		c = 0;
	else
		c = (char)s[0];
}

} }



#endif /* STRING_CONVERT_H_ */
