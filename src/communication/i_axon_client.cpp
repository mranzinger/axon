#include "communication/messaging/i_axon_client.h"

#if _WIN32
#define __thread_local __declspec(thread)
#else
#define __thread_local __thread
#endif

namespace axon { namespace communication {

__thread_local IAxonClient *s_executing = nullptr;

IAxonClient *IAxonClient::GetExecutingInstance()
{
    return s_executing;
}

void IAxonClient::SetExecutingInstance(IAxonClient *a_instance)
{
    s_executing = a_instance;
}

} }
