/*
 * enum_to_string.h
 *
 *  Created on: Feb 9, 2014
 *      Author: Mike
 */

#ifndef ENUM_TO_STRING_H_
#define ENUM_TO_STRING_H_

#include <map>
#include <string>
#include <stdexcept>
#include <iostream>
#include "string_convert.h"

#include "detail/make_unique.h"

namespace axon { namespace util {

namespace EnumIOInternal {

template<typename T>
class enum_io_helper
{
public:
	typedef std::pair<T, std::string> pair_type;
	typedef std::map<T, std::string> list_type;

	enum_io_helper() : _hasDefault(false) { }

	// This operator will add a new mapping to the list
	enum_io_helper &operator()(T val, const char *name)
	{
		_map[val] = name;

		return *this;
	}
	enum_io_helper &operator()(const char *name, T val)
	{
		_map[val] = name;

		return *this;
	}

	// This operator overload will initialize the default value
	enum_io_helper &operator()(T val)
	{
		_default = val;
		_hasDefault = true;
		return *this;
	}

	const std::string &to_string(T val) const
	{
		auto iter = _map.find(val);

		if (iter != _map.end())
			return iter->second;

		if (!_hasDefault || val == _default)
			throw std::runtime_error("Unable to stringify this value because a default was not specified.");

		return to_string(_default);
	}

	T string_to(const std::string &s) const
	{
		for (auto iter = _map.begin(), end = _map.end(); iter != end; ++iter)
		{
			if (stricmp(iter->second.c_str(), s.c_str()) == 0)
				return iter->first;
		}

		if (_hasDefault)
			return _default;

		throw std::runtime_error("Unable to parse token because it did not match any value.");
	}

private:
	list_type _map;
	T _default;
	bool _hasDefault;
};

}

} }

#define ENUM_IO_MAP_SL(EnumType, varName) \
	extern const axon::util::EnumIOInternal::enum_io_helper<EnumType> varName; \
	\
	inline std::ostream &operator<<(std::ostream &out, EnumType e) \
{ return out << varName.to_string(e); } \
	inline std::istream &operator>>(std::istream &in, EnumType &e) \
{ std::string s; \
	in >> s; \
	e = varName.string_to(s); \
	return in; } \
	namespace axon { namespace util {\
	template<> inline EnumType StringTo<EnumType>(const std::string &s) { return varName.string_to(s); } \
	template<> inline std::string ToString<EnumType>(const EnumType &val) { return varName.to_string(val); } \
	__declspec(selectany) const EnumIOInternal::enum_io_helper<EnumType> varName = EnumIOInternal::enum_io_helper<EnumType>(); \
	} }

#define ENUM_IO_MAP(EnumType) ENUM_IO_MAP_SL(EnumType, MAKE_UNIQUE(__enumIO_))

#define ENTRY(EnumValue) (EnumValue, #EnumValue)
#define ENMAP(EnumValue, EnumString) (EnumValue, EnumString)
#define DEFAULT(EnumValue) (EnumValue)(EnumValue, #EnumValue)
#define DEFMAP(EnumValue, EnumString) (EnumValue)(EnumValue, EnumString)

#define ENUM_IO_FWD(enumType) \
	std::ostream &operator<<(std::ostream &out, enumType tp); \
	std::istream &operator>>(std::istream &in, enumType &tp)



#endif /* ENUM_TO_STRING_H_ */
