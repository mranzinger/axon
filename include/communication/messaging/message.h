/*
 * message.h
 *
 *  Created on: Feb 9, 2014
 *      Author: Mike
 */

#ifndef MESSAGE_H_
#define MESSAGE_H_

#include <type_traits>

#include "../fault_exception.h"
#include "serialization/master.h"
#include "util/buffer.h"
#include "util/enum_to_string.h"

namespace axon { namespace communication {

enum class MessageType
{
	Normal,
	Fault
};

ENUM_IO_FWD(MessageType);

class CMessage
{
private:
	serialization::CStructData::Ptr m_message;
	mutable serialization::CStringData *m_action = nullptr;
	mutable serialization::CStringData *m_id = nullptr;
	mutable serialization::CStringData *m_requestId = nullptr;
	mutable serialization::CBoolData *m_isOneWay = nullptr;

public:
	typedef std::shared_ptr<CMessage> Ptr;

	CMessage(MessageType a_type = MessageType::Normal);
	CMessage(serialization::AData::Ptr a_msgVal);
	CMessage(const CMessage &a_other, MessageType a_type);
	CMessage(const CMessage &a_other, const std::exception &a_ex);
	CMessage(const CMessage &a_other, const CFaultException &a_ex);

	const std::string &GetAction() const;
	void SetAction(const std::string &action);

	const std::string &Id() const;
	void SetId(const std::string &id);

	const std::string &RequestId() const;
	void SetRequestId(const std::string &id);

	bool IsOneWay() const;
	void SetOneWay(bool oneWay);

	void FaultCheck() const;

	serialization::CStructData *Msg() const { return m_message.get(); }
	void SetMessage(serialization::CStructData::Ptr a_msg);

	MessageType Type() const;

	template<typename T>
	T GetField(const char *a_fieldName) const
	{
		return serialization::Deserialize<T>(m_message->Get(a_fieldName));
	}

private:
	void Init(MessageType a_type);
	void SetFault(const CFaultException &a_ex);

	template<typename T>
	void SetField(serialization::CPrimData<T> *&a_data, const char *a_fieldName, T a_val)
	{
		if (a_data)
			a_data->SetValue(std::move(a_val));
		else
		{
			auto l_prim = serialization::MakePrim(a_val, m_message->Context());

			a_data = l_prim.get();

			m_message->Set(a_fieldName, std::move(l_prim));
		}
	}

	template<typename DT>
	auto TryGetField(DT *&a_vp, const char *a_name) const -> decltype(a_vp->GetValue())
	{
		typedef typename std::remove_const<
			typename std::remove_reference<decltype(a_vp->GetValue())>::type>::type
			T;

		static const T EMPTY_VAL = T();

		if (!a_vp)
		{
			auto l_sval = m_message->Find(a_name);

			if (!l_sval)
				return EMPTY_VAL;
			else
				a_vp = dynamic_cast<DT*>(l_sval);

			if (!a_vp)
				throw std::runtime_error("Invalid target data type.");
		}

		return a_vp->GetValue();
	}
};

} }

#endif /* MESSAGE_H_ */
