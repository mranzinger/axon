/*
 * File description: poly_manager.h
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#ifndef POLY_MANAGER_H_
#define POLY_MANAGER_H_

#include <typeindex>
#include <unordered_map>

#include "struct_binder.h"

namespace axon { namespace serialization {

class CPolyBase
{
public:
	typedef std::shared_ptr<CPolyBase> Ptr;

	virtual ~CPolyBase() { }

	virtual AData::Ptr WriteStructV(const void *a_ptr) const = 0;
	virtual void ReadStructV(const CStructReader &a_reader, void *&a_ptr) const = 0;
};

template<typename Base>
class CPolyBaseBase
	: public CPolyBase
{
public:
	virtual void WriteStructB(const CStructWriter &a_writer, const Base *a_ptr) const = 0;
	virtual void ReadStructB(const CStructReader &a_reader, Base *&a_ptr) const = 0;

private:
	virtual void WriteStructV(const CStructWriter &a_writer, const void *a_ptr) const override
	{
		const Base *a_base = reinterpret_cast<const Base *>(a_ptr);

		WriteStructB(a_writer, a_base);
	}
	virtual void ReadStructV(const CStructReader &a_reader, void *&a_ptr) const override
	{
		Base *a_base = nullptr;
		ReadStructB(a_reader, a_base);

		a_ptr = a_base;
	}
};

template<typename Base, typename Derived>
class CPolyImpl
	: public CPolyBaseBase<Base>
{
public:
	typedef std::shared_ptr<CPolyImpl> Ptr;

	virtual void WriteStructB(const CStructWriter &a_writer, const Base *a_ptr) const override
	{
		const Derived *a_der = dynamic_cast<const Derived *>(a_ptr);

		if (!a_der)
			throw std::runtime_error("Invalid usage of a derived class.");

		WriteStruct(a_writer, *a_der);
	}
	virtual void ReadStructB(const CStructReader &a_reader, Base *&a_ptr) const override
	{
		Derived *a_der = nullptr;
		ReadStruct(a_reader, a_der);

		a_ptr = a_der;
	}
};

class CPolyManager
{
private:
	std::unordered_map<std::string, CPolyBase::Ptr> m_nameLookup;
	std::unordered_map<std::type_index, CPolyBase::Ptr> m_typeLookup;

public:
	template<typename Base, typename Derived>
	static void Register()
	{
		p_GetInstance()->p_Register<Base, Derived>();
	}

	template<typename Base>
	static AData::Ptr WriteStruct(const Base *a_base, const CSerializationContext &a_context)
	{
		return p_GetInstance()->p_WriteStruct(a_base);
	}

	template<typename Base>
	static void ReadStruct(const CStructReader &a_reader, Base *&a_base)
	{
		p_GetInstance()->p_ReadStruct(a_reader, a_base);
	}

private:
	CPolyManager() { }
	CPolyManager(const CPolyManager &) = delete;
	CPolyManager &operator=(const CPolyManager &) = delete;

	static CPolyManager *p_GetInstance();

	template<typename Base, typename Derived>
	void p_Register()
	{
		auto l_ptr = std::make_shared<CPolyImpl<Base, Derived>>();

		std::type_index l_idx(typeid(Derived));

		m_nameLookup[p_GetPolyName(l_idx)] = l_ptr;
		m_typeLookup[l_idx] = std::move(l_ptr);
	}

	template<typename Base>
	AData::Ptr p_WriteStruct(const Base *a_base, const CSerializationContext &a_context) const
	{
		std::type_index idx(typeid(a_base));

		auto iter = m_typeLookup.find(idx);

		if (iter == m_typeLookup.end())
			throw std::runtime_error("Unable to find a matching serializer for the specified polymorphic type.");

		a_writer("_realType", p_GetPolyName(idx));

		iter->second->WriteStructV(a_writer, a_base);
	}

	template<typename Base>
	void p_ReadStruct(const CStructReader &a_reader, Base *&a_base) const
	{
		std::string l_realType = a_reader.GetPrimitive<std::string>("_realType");

		auto iter = m_nameLookup.find(l_realType);

		if (iter == m_nameLookup.end())
			throw std::runtime_error("Unable to find a matching serializer for the specified type name.");

		void *l_void = nullptr;
		iter->second->ReadStructV(a_reader, l_void);

		a_base = reinterpret_cast<Base*>(l_void);
	}
	std::string p_GetPolyName(const std::type_index &a_idx) const
	{
		return a_idx.name();
	}
};

} }



#endif /* POLY_MANAGER_H_ */
