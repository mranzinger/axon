/*
 * File description: axon_protocol.h
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#ifndef AXON_PROTOCOL_H_
#define AXON_PROTOCOL_H_

#include "a_state_protocol.h"
#include "serialization/format/a_serializer.h"

namespace axon { namespace communication {

enum class APState;

class CAxonProtocol
	: public AStateProtocol
{
private:
	APState m_currState;
	char m_lenHeader[4];
	char m_crcHeader[8];
	char m_crcData[8];
	CDataBuffer m_dataBuff;
	size_t m_stateCurr;

	serialization::ASerializer::Ptr m_serializer;

public:
	typedef std::shared_ptr<CAxonProtocol> Ptr;

	CAxonProtocol();
	CAxonProtocol(serialization::ASerializer::Ptr a_serializer);

	void SetSerializer(serialization::ASerializer::Ptr a_serializer);

	virtual CDataBuffer SerializeMessage(const CMessage &a_msg) const override;

protected:
	virtual void ProcessInternal(CDataBuffer a_buffer) override;

private:
	void p_FindAnchor(char *&a_curr, char *a_end);
	void p_ProcMsgHeader(char *&a_curr, char *a_end);
	void p_ProcCRCMsgHeader(char *&a_curr, char *a_end);
	void p_ProcCRCDataHeader(char *&a_curr, char *a_end);
	void p_ProcMessage(char *&a_curr, char *a_end);

	bool p_ReadIntoBuffer(char *a_target, size_t a_targetSize,
						  char *&a_curr, char *a_end);

	void p_ValidateHeader(size_t a_headerSize);
	void p_ValidateData();
	void p_Finalize();
	void p_MoveTo(APState a_state);

	void OnFinished(const CMessage::Ptr &a_msg);
	void ResetState();
};

} }




#endif /* AXON_PROTOCOL_H_ */
