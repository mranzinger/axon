/*
 * struct_data.cpp
 *
 *  Created on: Aug 4, 2014
 *      Author: mranzinger
 */

#include "base/struct_data.h"

#include <sstream>

using namespace std;

namespace axon { namespace serialization {

CStructData::CStructData()
    : AData(DataType::Struct)
{
}

CStructData::CStructData(CSerializationContext a_context)
    : AData(DataType::Struct, move(a_context))
{
}

void CStructData::Add(string a_name, AData::Ptr a_val)
{
    m_props.emplace_back(move(a_name), move(a_val));
}

void CStructData::Set(const string &a_name, AData::Ptr a_val)
{
    auto iter = find_if(m_props.begin(), m_props.end(),
            [&a_name] (const TProp &a_prop)
            {
                return a_name == a_prop.first;
            });

    if (iter == m_props.end())
        Add(a_name, move(a_val));
    else
        iter->second = move(a_val);
}

const AData::Ptr &CStructData::Get(const string &a_name) const
{
    static const AData::Ptr s_missing;

    auto iter = find_if(m_props.begin(), m_props.end(),
            [&a_name] (const TProp &a_prop)
            {
                return a_name == a_prop.first;
            });

    if (iter != m_props.end())
        return iter->second;
    else
        return s_missing;
}

AData *CStructData::Find(const string &a_name) const
{
    auto iter = find_if(m_props.begin(), m_props.end(),
            [&a_name] (const TProp &a_prop)
            {
                return a_name == a_prop.first;
            });

    if (iter != m_props.end())
        return iter->second.get();
    else
        return nullptr;
}

string CStructData::ToJsonString(size_t a_numSpaces) const
{
    ostringstream oss;
    oss << "{" << endl;

    string l_spaces(a_numSpaces + 4, ' ');

    for (auto iter = m_props.begin(), end = m_props.end(); iter != end; ++iter)
    {
        const TProp &l_prop = *iter;

        oss << l_spaces << "\"" <<
               l_prop.first << "\" : " <<
               l_prop.second->ToJsonString(a_numSpaces + l_prop.first.size() + 4);

        if (iter != (end - 1))
            oss << ", ";
        oss << endl;
    }
    oss << string(a_numSpaces, ' ') << "}";
    return oss.str();
}

} }
