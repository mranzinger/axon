/*
 * message.cpp
 *
 *  Created on: Feb 9, 2014
 *      Author: Mike
 */

#include "messaging/message.h"
#include "messaging/fault_serialization.h"

#include "util/enum_to_string.h"

using namespace std;
using namespace axon::serialization;
using namespace axon::communication;

ENUM_IO_MAP(MessageType)
	ENMAP(MessageType::Fault, "Fault")
	ENMAP(MessageType::Normal, "Normal");

namespace axon { namespace communication {

CMessage::CMessage(MessageType a_type)
{
	Init(a_type);
}

CMessage::CMessage(AData::Ptr a_msgVal)
{
	if (a_msgVal->Type() != DataType::Struct)
		throw runtime_error("The serialization object must be of struct type.");

	CStructData::Ptr l_struct(static_cast<CStructData*>(a_msgVal.release()));

	SetMessage(move(l_struct));
}

CMessage::CMessage(const CMessage &a_other)
{
    Init(a_other.Type());

    SetAction(a_other.GetAction());
    SetId(a_other.Id());
    SetRequestId(a_other.RequestId());
    SetOneWay(a_other.IsOneWay());
}

CMessage::CMessage(const CMessage& a_other, MessageType a_type)
{
	Init(a_type);

	SetRequestId(a_other.Id());
}

CMessage::CMessage(const CMessage& a_other, const std::exception& a_ex)
	: CMessage(a_other, CFaultException(a_ex.what()))
{

}

CMessage::CMessage(const CMessage& a_other, const CFaultException& a_ex)
{
	Init(MessageType::Fault);

	SetFault(a_ex);

	SetRequestId(a_other.Id());
}

void CMessage::Init(MessageType a_type)
{
	static const string EMPTY_STRING;

	m_message.reset(new CStructData);

	SetId(EMPTY_STRING);
	SetAction(EMPTY_STRING);
	SetOneWay(false);

	m_message->Add("Type", MakePrim(axon::util::ToString(a_type), m_message->Context()));
}

const std::string& CMessage::GetAction() const
{
	return TryGetField(m_action, "Action");
}

const std::string& CMessage::Id() const
{
	return TryGetField(m_id, "Id");
}

const std::string& CMessage::RequestId() const
{
	return TryGetField(m_requestId, "RequestId");
}

bool CMessage::IsOneWay() const
{
	return TryGetField(m_isOneWay, "OneWay");
}

void CMessage::SetAction(const std::string& action)
{
	SetField(m_action, "Action", action);
}

void CMessage::SetId(const std::string& id)
{
	SetField(m_id, "Id", id);
}

void CMessage::SetRequestId(const std::string& id)
{
	SetField(m_requestId, "RequestId", id);
}

void CMessage::SetOneWay(bool oneWay)
{
	SetField(m_isOneWay, "OneWay", oneWay);
}

MessageType CMessage::Type() const
{
	AData *l_pType = m_message->Find("Type");

	if (!l_pType)
		return MessageType::Normal;

	return util::StringTo<MessageType>(static_cast<CStringData*>(l_pType)->GetValue());
}

void CMessage::SetFault(const CFaultException& a_ex)
{
	m_message->Set("Fault", Serialize(a_ex));
}

void CMessage::FaultCheck() const
{
	AData *l_pFault = m_message->Find("Fault");

	if (!l_pFault)
		return;

	CFaultException l_ex;
	Deserialize(*l_pFault, l_ex);

	throw l_ex;
}

void CMessage::SetMessage(serialization::CStructData::Ptr a_msg)
{
	m_message = move(a_msg);
	m_id = nullptr;
	m_action = nullptr;
	m_requestId = nullptr;
	m_isOneWay = nullptr;
}







}
}


