#include "communication/messaging/axon_protocol_compressed.h"

using namespace std;
using namespace axon::util;
using namespace axon::serialization;

namespace axon { namespace communication {

CAxonProtocolCompressed::CAxonProtocolCompressed()
{
    SetCompressor(ICompressor::DEFAULT_INSTANCE);

    SetHandler(bind(&CAxonProtocolCompressed::OnOuterProcessed, this, placeholders::_1));
}

CAxonProtocolCompressed::CAxonProtocolCompressed(ASerializer::Ptr a_serializer)
    : CAxonProtocolCompressed() 
{
    m_inner.SetSerializer(move(a_serializer));
}

void CAxonProtocolCompressed::SetCompressor(ICompressor::Ptr a_compressor)
{
    m_compressor = move(a_compressor);
}

CDataBuffer CAxonProtocolCompressed::SerializeMessage(const CMessage &a_msg) const
{
    CDataBuffer l_inner = m_inner.SerializeMessage(a_msg);
   
    std::unique_ptr<char[]> l_compData;
    size_t l_compSize;
    m_compressor->Compress(l_inner.data(), l_inner.size(), l_compData, l_compSize); 

    CBuffer l_sb(l_compSize, l_compData.release(), CBuffer::TakeOwnership);

    CMessage l_msgOuter(a_msg);
    l_msgOuter.Add("Compressed", Serialize(l_sb));

    return m_outer.SerializeMessage(l_msgOuter);
}

void CAxonProtocolCompressed::Process(CDataBuffer a_buffer)
{
    m_outer.Process(move(a_buffer));
}

void CAxonProtocolCompressed::OnOuterProcessed(const CMessage::Ptr &a_outerMsg)
{
    auto l_buff = a_outerMsg->GetField<CDataBuffer>("Compressed"); 

    m_inner.Process(move(l_buff));
}

} }


