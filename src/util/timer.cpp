/*
 * timer.cpp
 *
 *  Created on: Dec 22, 2014
 *      Author: mranzinger
 */

#include "util/timer.h"

#ifdef IS_WINDOWS

#else
#include "detail/posix_timer.h"
#endif

namespace axon { namespace util {

Timer::Timer()
{
    m_impl.reset(new Impl());
}

Timer::Timer(std::chrono::milliseconds a_interval,
             bool a_recurring)
{
    m_impl.reset(new Impl(a_interval, a_recurring));
}

Timer::~Timer()
{
}

void Timer::SetInterval(std::chrono::milliseconds a_interval, bool a_recurring)
{
    m_impl->SetInterval(a_interval, a_recurring);
}

void Timer::SetCallback(TCallback a_handler)
{
    m_impl->SetCallback(std::move(a_handler));
}

void Timer::Start()
{
    m_impl->Start();
}

void Timer::Stop()
{
    m_impl->Stop();
}

}
}


