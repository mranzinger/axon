/*
 * File description: poly_manager.h
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#ifndef POLY_MANAGER_H_
#define POLY_MANAGER_H_

#include <memory>
#include <typeindex>
#include <unordered_map>
#include <stdexcept>

#include "struct_binder.h"

namespace axon { namespace serialization {

class AXON_SERIALIZE_API CPolyBase
{
public:
	typedef std::shared_ptr<CPolyBase> Ptr;

	virtual ~CPolyBase() { }

	virtual const std::string &GetTypeName() const = 0;

	virtual void WriteStructV(const CStructWriter &a_writer, const void *a_ptr) const = 0;
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
		Base *a_base = (Base*)a_ptr;
		ReadStructB(a_reader, a_base);

		a_ptr = a_base;
	}
};

template<typename Base, typename Derived>
class CPolyImpl
	: public CPolyBaseBase<Base>
{
private:
	std::string m_typeName;

public:
	typedef std::shared_ptr<CPolyImpl> Ptr;

	CPolyImpl(std::string a_typeName)
		: m_typeName(std::move(a_typeName)) { }

	virtual ~CPolyImpl() { }

	virtual const std::string &GetTypeName() const { return m_typeName; }

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
		if (a_ptr)
		{
			a_der = dynamic_cast<Derived*>(a_ptr);
			if (!a_der)
				throw std::runtime_error("Invalid default instantiation of pointer.");
		}
		else
		{
			a_der = new Derived{};
		}

		ReadStruct(a_reader, *a_der);

		a_ptr = a_der;
	}
};

class AXON_SERIALIZE_API CPolyManager
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

	template<typename Base, typename Derived>
	static void Register(std::string a_typeName)
	{
		p_GetInstance()->p_Register<Base, Derived>(std::move(a_typeName));
	}

	template<typename Base>
	static void WriteStruct(const CStructWriter &a_writer, const Base *a_base)
	{
		return p_GetInstance()->p_WriteStruct(a_writer, a_base);
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
		p_Register<Base, Derived>(std::string{});
	}

	template<typename Base, typename Derived>
	void p_Register(std::string a_typeName)
	{
		auto l_ptr = std::make_shared<CPolyImpl<Base, Derived>>(a_typeName);

		std::type_index l_idx(typeid(Derived));

		if (a_typeName.empty())
			a_typeName = p_GetPolyName(l_idx);

		m_nameLookup.emplace(std::move(a_typeName), l_ptr);
		m_typeLookup.emplace(l_idx, std::move(l_ptr));
	}

	template<typename Base>
	void p_WriteStruct(const CStructWriter &a_writer, const Base *a_base) const
	{
		std::type_index idx(typeid(*a_base));

		auto iter = m_typeLookup.find(idx);

		if (iter == m_typeLookup.end())
			throw std::runtime_error("Unable to find a matching serializer for the specified polymorphic type.");

		a_writer("_realType", iter->second->GetTypeName());

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

template<typename T, bool Supported>
struct CPolymorphicBinderBase
{
	static const bool specialized = false;

	static void Write(const CStructWriter &a_writer, const T *a_val);
	static void Read(const CStructReader &a_reader, T *&a_val);
};

template<typename T>
struct CPolymorphicBinderBase<T, true>
{
	static const bool specialized = true;

	static void Write(const CStructWriter &a_writer, const T *a_val)
	{
		CPolyManager::WriteStruct(a_writer, a_val);
	}
	static void Read(const CStructReader &a_reader, T *&a_val)
	{
		CPolyManager::ReadStruct(a_reader, a_val);
	}
};

template<typename T>
struct CPolymorphicBinder
	: CPolymorphicBinderBase<T, false>
{

};

#define AXON_SERIALIZE_BASE_TYPE(type) \
	namespace axon { namespace serialization { template<> struct CPolymorphicBinder<type> : CPolymorphicBinderBase<type, true> { }; } }

template<typename Base, typename Derived>
struct CRegisterDerived
{
	CRegisterDerived(std::string a_typeName = "")
	{
		CPolyManager::Register<Base, Derived>(std::move(a_typeName));
	}
};

#define AXON_SERIALIZE_DERIVED_TYPE(baseType, derType, name) \
		static const ::axon::serialization::CRegisterDerived<baseType, derType> __axon_derived_register_ ## baseType ## _ ## derType (name)

#define AXON_SERIALIZE_DERIVED_TYPE_D(baseType, derType) AXON_SERIALIZE_DERIVED_TYPE(baseType, derType, "")


} }



#endif /* POLY_MANAGER_H_ */
