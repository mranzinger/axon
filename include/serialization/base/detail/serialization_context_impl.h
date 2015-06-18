/*
 * File description: serialization_context_impl.h
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#ifndef SERIALIZATION_CONTEXT_IMPL_H_
#define SERIALIZATION_CONTEXT_IMPL_H_

#include <vector>
#include <map>
#include <string>
#include <algorithm>

namespace axon { namespace serialization {

class CSerializationContext;

namespace detail {

class CSerializationContextImpl
{
friend class ::axon::serialization::CSerializationContext;

    typedef std::pair<std::string, std::string> TVariable;
    typedef std::vector<TVariable> TVariableMap;

	// Everything is private so that only CSerializationContext can
	// actually manipulate this object
private:
	mutable int m_refCt;
    TVariableMap m_varMap;

	CSerializationContextImpl() : m_refCt(1) { }

	void AddRef() { ++m_refCt; }
	void RemoveRef()
	{
		--m_refCt;

		if (0 == m_refCt)
			delete this;
	}

    std::string *GetVariable(const std::string &a_name)
    {
        auto iter = std::find_if(std::begin(m_varMap), std::end(m_varMap),
            [&a_name] (const TVariable &v) { return a_name == v.first; });

        if (iter == std::end(m_varMap))
            return nullptr;
        else
            return &iter->second;
    }
    void SetVariable(const std::string &a_name, std::string a_val)
    {
        std::string *st = GetVariable(a_name);
        if (st)
            *st = std::move(a_val);
        else
            m_varMap.emplace_back(a_name, std::move(a_val));
    }
};

} // detail

} }



#endif /* SERIALIZATION_CONTEXT_IMPL_H_ */
