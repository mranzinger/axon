/*
 * array_data.cpp
 *
 *  Created on: Aug 4, 2014
 *      Author: mranzinger
 */

#include "base/array_data.h"

namespace axon { namespace serialization {

CArrayData::CArrayData()
        : AData(DataType::Array) { }

CArrayData::CArrayData(CSerializationContext a_context)
        : AData(DataType::Array, std::move(a_context)) { }

void CArrayData::Add(AData::Ptr a_val)
{
    m_children.push_back(std::move(a_val));
}

void CArrayData::Erase(size_t idx)
{
    m_children.erase(m_children.begin() + idx);
}

const AData::Ptr& CArrayData::operator [](size_t idx) const
{
    return m_children[idx];
}

const AData::Ptr& CArrayData::Get(size_t idx) const
{
    return m_children[idx];
}

std::string CArrayData::ToJsonString(size_t a_numSpaces) const
{
    if (m_children.empty())
    return "[]";

    std::ostringstream oss;
    oss << "[" << std::endl;
    std::string l_innerWhite(a_numSpaces + 4, ' ');
    oss << l_innerWhite << m_children[0]->ToJsonString(a_numSpaces + 4);
    for (auto iter = m_children.cbegin() + 1, end = m_children.end();iter != end;++iter)
    {
        const AData::Ptr& l_child = *iter;
        oss << "," << std::endl << l_innerWhite << l_child->ToJsonString(a_numSpaces + 4);
    }
    oss << std::endl << std::string(a_numSpaces, ' ') << "]";
    return oss.str();
}

} }

