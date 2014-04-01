/*
 * File description: axon_protocol.cpp
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#include "communication/messaging/axon_protocol.h"

#include "serialization/format/axon_serializer.h"

#include <algorithm>

using namespace std;

using namespace axon::serialization;

namespace axon { namespace communication {

const char s_specialToken = 172;

enum class APState
{
	Anchor,
	MsgHeader,
	CRCMsgHeader,
	CRCDataHeader,
	Message
};

CAxonProtocol::CAxonProtocol()
{
	ResetState();

	SetSerializer(make_shared<CAxonSerializer>());
}

CAxonProtocol::CAxonProtocol(ASerializer::Ptr a_serializer)
{
	ResetState();

	SetSerializer(move(a_serializer));
}

void CAxonProtocol::SetSerializer(ASerializer::Ptr a_serializer)
{
	if (!a_serializer)
		throw runtime_error("Cannot set the serializer to null.");

	m_serializer = move(a_serializer);
}

CDataBuffer CAxonProtocol::SerializeMessage(const CMessage& a_msg) const
{
	const size_t l_msgSize = m_serializer->CalcSize(*a_msg.Msg());

	if (l_msgSize > numeric_limits<uint32_t>::max())
		throw runtime_error("The message size cannot exceed 4 GiB.");

	const size_t l_bufferSize = 1 + sizeof(m_lenHeader) +
					  sizeof(m_crcHeader) + sizeof(m_crcData) +
					  l_msgSize;

	CDataBuffer l_ret(l_bufferSize);

	char *l_opBuff = l_ret.Data();
	l_opBuff[0] = s_specialToken;
	memcpy(l_opBuff + 1, &l_msgSize, sizeof(m_lenHeader));

	// TODO: Add the CRC mechanisms

	m_serializer->SerializeInto(*a_msg.Msg(),
			l_opBuff + 1 + sizeof(m_lenHeader) + sizeof(m_crcHeader) + sizeof(m_crcData),
			l_msgSize);

	return move(l_ret);
}

void CAxonProtocol::ProcessInternal(CDataBuffer a_buffer)
{
	if (a_buffer.Size() == 0)
		return;

	char *l_curr = a_buffer.Data();
	char *l_end = a_buffer.end();

	while (l_curr && l_curr < l_end)
	{
		switch (m_currState)
		{
		case APState::Anchor:
			p_FindAnchor(l_curr, l_end);
			break;
		case APState::MsgHeader:
			p_ProcMsgHeader(l_curr, l_end);
			break;
		case APState::CRCMsgHeader:
			p_ProcCRCMsgHeader(l_curr, l_end);
			break;
		case APState::CRCDataHeader:
			p_ProcCRCDataHeader(l_curr, l_end);
			break;
		case APState::Message:
			p_ProcMessage(l_curr, l_end);
			break;
		}
	}
}

void CAxonProtocol::p_FindAnchor(char *&a_curr, char *a_end)
{
	for (; a_curr != a_end; ++a_curr)
	{
		if (*a_curr == s_specialToken)
		{
			++a_curr;
			p_MoveTo(APState::MsgHeader);
			break;
		}
	}
}

void CAxonProtocol::p_ProcMsgHeader(char*& a_curr, char* a_end)
{
	if (p_ReadIntoBuffer(m_lenHeader, sizeof(m_lenHeader), a_curr, a_end))
	{
		p_MoveTo(APState::CRCMsgHeader);
	}
}

void CAxonProtocol::p_ProcCRCMsgHeader(char*& a_curr, char* a_end)
{
	if (p_ReadIntoBuffer(m_crcHeader, sizeof(m_crcHeader), a_curr, a_end))
	{
		size_t l_headerSize = 0;
		memcpy(&l_headerSize, m_lenHeader, sizeof(m_lenHeader));

		p_ValidateHeader(l_headerSize);

		m_dataBuff.Reset(l_headerSize);

		p_MoveTo(APState::CRCDataHeader);
	}
}

void CAxonProtocol::p_ProcCRCDataHeader(char*& a_curr, char* a_end)
{
	if (p_ReadIntoBuffer(m_crcData, sizeof(m_crcData), a_curr, a_end))
	{
		p_MoveTo(APState::Message);
	}
}

void CAxonProtocol::p_ProcMessage(char*& a_curr, char* a_end)
{
	if (p_ReadIntoBuffer(m_dataBuff.Data(), m_dataBuff.Size(),
			a_curr, a_end))
	{
		p_ValidateData();

		p_Finalize();

		p_MoveTo(APState::Anchor);
	}
}

void CAxonProtocol::FinishProcessing(CDataBuffer a_buffer)
{
 	try
	{
		AData::Ptr l_data = m_serializer->DeserializeData(
				a_buffer.begin(), a_buffer.end());

		auto l_msg = make_shared<CMessage>(move(l_data));

		OnFinished(l_msg);
	}
	catch (...)
	{
		// TODO: Log this?
	}   
}

void CAxonProtocol::p_Finalize()
{
    FinishProcessing(move(m_dataBuff));
}

bool CAxonProtocol::p_ReadIntoBuffer(char* a_target, size_t a_targetSize,
		char*& a_curr, char* a_end)
{
	size_t l_numRead = min<size_t>(a_targetSize - m_stateCurr, a_end - a_curr);

	memcpy(a_target + m_stateCurr, a_curr, l_numRead);
	a_curr += l_numRead;
	m_stateCurr += l_numRead;

	return m_stateCurr == a_targetSize;
}

void CAxonProtocol::p_MoveTo(APState a_state)
{
	m_stateCurr = 0;
	m_currState = a_state;
}

void CAxonProtocol::p_ValidateHeader(size_t a_headerSize)
{
}

void CAxonProtocol::p_ValidateData()
{
}

void CAxonProtocol::OnFinished(const CMessage::Ptr& a_msg)
{
	MessageProcessed(a_msg);

	ResetState();
}

void CAxonProtocol::ResetState()
{
	p_MoveTo(APState::Anchor);
}

} }
