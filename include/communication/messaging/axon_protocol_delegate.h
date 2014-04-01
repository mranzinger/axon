#ifndef _AXON_PROTOCOL_DELEGATE_H_
#define _AXON_PROTOCOL_DELEGATE_H_

#include <function>

#include "axon_protocol.h"

namespace axon { namespace communication {

class AXON_COMMUNICATEi_API CAxonProtocolDelegate
    : public CAxonProtocol
{
public:
    typedef std::function<void (CDataBuffer)> TFinishProcFn;

private:
    TFinishProcFn m_procFn;

public:
    typedef std::unique_ptr<CAxonProtocolDelegate> Ptr;

    CAxonProtocolDelegate() { }
    CAxonProtocolDelegate(TFinishProcFn a_fn)
        : m_procFn(std::move(a_fn))
    {

    }

    void SetProcHandler(TFinishProcFn a_fn)
    {
        m_procFn = std::move(a_fn);
    }

protected:
    virtual void FinishProcessing(CDataBuffer a_buffer) override
    {
        m_procFn(std::move(a_buffer));
    }
};

} }

#endif

