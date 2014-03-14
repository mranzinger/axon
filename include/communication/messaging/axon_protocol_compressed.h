#ifndef _AXON_PROTOCOL_CHAIN_H_
#define _AXON_PROTOCOL_CHAIN_H_

#include "axon_protocol.h"
#include "serialization/format/i_compressor.h"

namespace axon { namespace communication {

class AXON_COMMUNICATE_API CAxonProtocolCompressed
    : public AProtocol
{
private:
    CAxonProtocol m_inner;
    CAxonProtocol m_outer;
    serialization::ICompressor::Ptr m_compressor;

public:
    CAxonProtocolCompressed();
    CAxonProtocolCompressed(serialization::ASerializer::Ptr a_serializer);
            
    void SetCompressor(serialization::ICompressor::Ptr a_compressor);

    virtual CDataBuffer SerializeMessage(const CMessage &a_msg) const override;

    virtual void Process(CDataBuffer a_buffer) override;

private:
    void OnOuterProcessed(const CMessage::Ptr &a_msg);
};

} }

#endif

