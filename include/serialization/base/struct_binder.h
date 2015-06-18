/*
 * File description: struct_binder.h
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#ifndef STRUCT_BINDER_H_
#define STRUCT_BINDER_H_

#include "a_data.h"
#include "prim_data.h"
#include "array_data.h"
#include "struct_data.h"

namespace axon { namespace serialization {

namespace detail {

struct unspecialized { };

}

class CStructBinder;
class CStructWriter;
class CStructReader;

template<typename T>
detail::unspecialized BindStruct(const CStructBinder &a_binder, T &a_val);

template<typename T>
auto WriteStruct(const CStructWriter &a_writer, const T &a_val) -> decltype(BindStruct(*(CStructBinder*)nullptr, *(T*)nullptr))
{
	return BindStruct(&a_writer, const_cast<T&>(a_val));
}

template<typename T>
auto ReadStruct(const CStructReader &a_reader, T &a_val) -> decltype(BindStruct(*(CStructBinder*)nullptr, *(T*)nullptr))
{
	return BindStruct(&a_reader, a_val);
}


template<typename T>
AData::Ptr Serialize(const T &, const CSerializationContext &);
template<typename T>
T Deserialize(const AData &a_data);

class AXON_SERIALIZE_API CStructWriter
{
private:
	CStructData *m_data;

public:
	CStructWriter(CStructData *a_data)
		: m_data(a_data) { }

	const CSerializationContext &GetContext() const {
		return m_data->Context();
	}

	template<typename T, typename ...Flags>
	const CStructWriter &operator()(std::string a_name, const T &a_val, Flags ...a_flags) const
	{
		const auto &l_flags = SetSerFlags(m_data->Context(), a_flags...);

		m_data->Add(std::move(a_name),
				    Serialize(a_val, m_data->Context()));

		return *this;
	}

	void Append(std::string a_name, AData::Ptr a_data) const
	{
		m_data->Add(std::move(a_name), std::move(a_data));
	}
};

class AXON_SERIALIZE_API CStructReader
{
private:
	const CStructData *m_data;

public:
	CStructReader(const CStructData *a_data)
		: m_data(a_data) { }

	const CSerializationContext &GetContext() const {
		return m_data->Context();
	}

	template<typename T>
	T GetPrimitive(const std::string &a_name) const
	{
		return Deserialize<T>(*m_data->Get(a_name));
	}

	const AData *GetData(const std::string &a_name) const
	{
		return m_data->Find(a_name);
	}

	template<typename T, typename ...Flags>
	const CStructReader &operator()(const std::string &a_name, T &a_val, Flags ...a_flags) const
	{
		const auto &l_flags = SetSerFlags(m_data->Context(), a_flags...);

		const AData *data = m_data->Find(a_name);

		if (data)
		{
		    Deserialize(*data, a_val);
		}

		return *this;
	}

	template<typename ObjType, typename T, typename ...Flags>
	const CStructReader &operator()(const std::string &a_name, ObjType &a_obj, void (ObjType::*mem_fun) (T), Flags ...a_flags) const
	{
	    const auto &l_flags = SetSerFlags(m_data->Context(), a_flags...);

	    const AData *data = m_data->Find(a_name);

	    if (data)
	    {
	        T l_val;
	        Deserialize(*data, l_val);

	        (a_obj.*mem_fun)(std::move(l_val));
	    }

	    return *this;
	}
	template<typename ObjType, typename T, typename ...Flags>
	const CStructReader &operator()(const std::string &a_name, ObjType &a_obj, void (ObjType::*mem_fun) (const T &), Flags ...a_flags) const
	{
	    const auto &l_flags = SetSerFlags(m_data->Context(), a_flags...);

        const AData *data = m_data->Find(a_name);

        if (data)
        {
            T l_val;
            Deserialize(*data, l_val);

            (a_obj.*mem_fun)(l_val);
        }

        return *this;
	}
};

class AXON_SERIALIZE_API CStructBinder
{
private:
	const CStructWriter *m_writer = nullptr;
	const CStructReader *m_reader = nullptr;

public:
	CStructBinder(const CStructWriter *a_writer) : m_writer(a_writer) { }
	CStructBinder(const CStructReader *a_reader) : m_reader(a_reader) { }

	bool IsRead() const { return m_reader; }
	bool IsWrite() const { return m_writer; }

	const CSerializationContext &GetContext() const {
	    if (m_writer)
            return m_writer->GetContext();
        else
            return m_reader->GetContext();
    }

	template<typename T, typename ...Flags>
	const CStructBinder &operator()(const std::string &a_name, T &a_val, Flags ...a_flags) const
	{
		if (m_writer)
			(*m_writer)(a_name, a_val, a_flags...);
		else
			(*m_reader)(a_name, a_val, a_flags...);

		return *this;
	}

	template<typename ObjType, typename T, typename ...Flags>
	const CStructBinder &operator()(const std::string &a_name, ObjType &a_obj,
	                                const T &(ObjType::*getter) () const,
	                                void (ObjType::*setter) (T),
	                                Flags ...a_flags) const
	{
	    if (m_writer)
	        (*m_writer)(a_name, (a_obj.*getter)(), a_flags...);
	    else
	        (*m_reader)(a_name, a_obj, setter, a_flags...);

	    return *this;
	}

	template<typename ObjType, typename T, typename ...Flags>
	const CStructBinder &operator()(const std::string &a_name, ObjType &a_obj,
	                                T (ObjType::*getter) () const,
	                                void (ObjType::*setter) (T),
	                                Flags ...a_flags) const
	{
	    if (m_writer)
            (*m_writer)(a_name, (a_obj.*getter)(), a_flags...);
        else
            (*m_reader)(a_name, a_obj, setter, a_flags...);

	    return *this;
	}

	template<typename ObjType, typename T, typename ...Flags>
    const CStructBinder &operator()(const std::string &a_name, ObjType &a_obj,
                                    const T &(ObjType::*getter) (),
                                    void (ObjType::*setter) (T),
                                    Flags ...a_flags) const
    {
        if (m_writer)
            (*m_writer)(a_name, (a_obj.*getter)(), a_flags...);
        else
            (*m_reader)(a_name, a_obj, setter, a_flags...);

        return *this;
    }

    template<typename ObjType, typename T, typename ...Flags>
    const CStructBinder &operator()(const std::string &a_name, ObjType &a_obj,
                                    T (ObjType::*getter) (),
                                    void (ObjType::*setter) (T),
                                    Flags ...a_flags) const
    {
        if (m_writer)
            (*m_writer)(a_name, (a_obj.*getter)(), a_flags...);
        else
            (*m_reader)(a_name, a_obj, setter, a_flags...);

        return *this;
    }

    template<typename ObjType, typename T, typename ...Flags>
    const CStructBinder &operator()(const std::string &a_name, ObjType &a_obj,
                                    const T &(ObjType::*getter) () const,
                                    void (ObjType::*setter) (const T &),
                                    Flags ...a_flags) const
    {
        if (m_writer)
            (*m_writer)(a_name, (a_obj.*getter)(), a_flags...);
        else
            (*m_reader)(a_name, a_obj, setter, a_flags...);

        return *this;
    }

    template<typename ObjType, typename T, typename ...Flags>
    const CStructBinder &operator()(const std::string &a_name, ObjType &a_obj,
                                    T (ObjType::*getter) () const,
                                    void (ObjType::*setter) (const T &),
                                    Flags ...a_flags) const
    {
        if (m_writer)
            (*m_writer)(a_name, (a_obj.*getter)(), a_flags...);
        else
            (*m_reader)(a_name, a_obj, setter, a_flags...);

        return *this;
    }

    template<typename ObjType, typename T, typename ...Flags>
    const CStructBinder &operator()(const std::string &a_name, ObjType &a_obj,
                                    const T &(ObjType::*getter) (),
                                    void (ObjType::*setter) (const T &),
                                    Flags ...a_flags) const
    {
        if (m_writer)
            (*m_writer)(a_name, (a_obj.*getter)(), a_flags...);
        else
            (*m_reader)(a_name, a_obj, setter, a_flags...);

        return *this;
    }

    template<typename ObjType, typename T, typename ...Flags>
    const CStructBinder &operator()(const std::string &a_name, ObjType &a_obj,
                                    T (ObjType::*getter) (),
                                    void (ObjType::*setter) (const T &),
                                    Flags ...a_flags) const
    {
        if (m_writer)
            (*m_writer)(a_name, (a_obj.*getter)(), a_flags...);
        else
            (*m_reader)(a_name, a_obj, setter, a_flags...);

        return *this;
    }

    template<typename ObjType, typename T, typename ...Flags>
    const CStructBinder &operator()(const std::string &a_name, ObjType &a_obj,
                                    T &(ObjType::*getter) (),
                                    Flags ...a_flags) const
    {
        if (m_writer)
            (*m_writer)(a_name, (a_obj.*getter)(), a_flags...);
        else
            (*m_reader)(a_name, (a_obj.*getter)(), a_flags...);

        return *this;
    }
};

} }


#endif /* STRUCT_BINDER_H_ */
