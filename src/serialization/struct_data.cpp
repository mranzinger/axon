/*
 * struct_data.cpp
 *
 *  Created on: Aug 4, 2014
 *      Author: mranzinger
 */

#include "base/struct_data.h"

namespace axon { namespace serialization {

CStructData::CStructData()
    : AData(DataType::Struct)
{
}

CStructData::CStructData(CSerializationContext a_context)
    : AData(DataType::Struct, std::move(a_context))
{
}

void CStructData::Add(std::string a_name, AData::Ptr a_val)
{
    m_props.emplace_back(std::move(a_name), std::move(a_val));
}

void CStructData::Set(const std::string &a_name, AData::Ptr a_val)
{
    auto iter = std::find_if(m_props.begin(), m_props.end(),
            [&a_name] (const TProp &a_prop)
            {
                return a_name == a_prop.first;
            });

    if (iter == m_props.end())
        Add(a_name, std::move(a_val));
    else
        iter->second = std::move(a_val);
}

const AData::Ptr &CStructData::Get(const std::string &a_name) const
{
    static const AData::Ptr s_missing;

    auto iter = std::find_if(m_props.begin(), m_props.end(),
            [&a_name] (const TProp &a_prop)
            {
                return a_name == a_prop.first;
            });

    if (iter != m_props.end())
        return iter->second;
    else
        return s_missing;
}

AData *CStructData::Find(const std::string &a_name) const
{
    auto iter = std::find_if(m_props.begin(), m_props.end(),
            [&a_name] (const TProp &a_prop)
            {
                return a_name == a_prop.first;
            });

    if (iter != m_props.end())
        return iter->second.get();
    else
        return nullptr;
}

std::string CStructData::ToJsonString(size_t a_numSpaces) const
{
    std::ostringstream oss;
    oss << "{" << std::endl;

    std::string l_spaces(a_numSpaces + 4, ' ');

    for (auto iter = m_props.begin(), end = m_props.end(); iter != end; ++iter)
    {
        const TProp &l_prop = *iter;

        oss << l_spaces << "\"" <<
               l_prop.first << "\" : " <<
               l_prop.second->ToJsonString(a_numSpaces + l_prop.first.size() + 4);

        if (iter != (end - 1))
            oss << ", ";
        oss << std::endl;
    }
    oss << std::string(a_numSpaces, ' ') << "}";
    return oss.str();
}

} }
