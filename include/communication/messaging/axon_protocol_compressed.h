#ifndef _AXON_PROTOCOL_CHAIN_H_
#define _AXON_PROTOCOL_CHAIN_H_

#include "axon_protocol.h"
#include "serialization/format/i_compressor.h"

namespace axon { namespace communication {

class AXON_COMMUNICATE_API CAxonProtocolCompressed
    : public IProtocol
{
private:
    CAxonProtocol m_inner;
    CAxonProtocol m_outer;
    serialization::ICompressor::Ptr m_compressor;

public:
    typedef std::unique_ptr<CAxonProtocolCompressed> Ptr;

    CAxonProtocolCompressed();
    CAxonProtocolCompressed(serialization::ASerializer::Ptr a_serializer);
            
    static Ptr Create()
    {
        return Ptr(new CAxonProtocolCompressed);
    }
    static Ptr Create(serialization::ASerializer::Ptr a_serializer)
    {
        return Ptr(new CAxonProtocolCompressed(std::move(a_serializer)));
    }

    virtual void SetHandler(HandlerFn a_fn) override;

    void SetCompressor(serialization::ICompressor::Ptr a_compressor);

    virtual CDataBuffer SerializeMessage(const CMessage &a_msg) const override;

    virtual void Process(CDataBuffer a_buffer) override;

private:
    void OnOuterProcessed(const CMessage::Ptr &a_msg);
};

} }

#endif

