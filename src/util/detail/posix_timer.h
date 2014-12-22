/*
 * posix_timer.h
 *
 *  Created on: Dec 22, 2014
 *      Author: mranzinger
 */

#pragma once

#include "util/timer.h"

#include <sys/time.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdexcept>

using namespace std;

namespace c = std::chrono;

namespace axon { namespace util {

class Timer::Impl
{
public:
    Impl();
    Impl(c::milliseconds a_interval, bool a_recurring);
    ~Impl();

    void SetInterval(c::milliseconds a_interval,
                     bool a_recurring);
    void SetCallback(TCallback a_callback);

    void Start();
    void Stop();

private:
    void p_Handler();

    static void p_StaticHandler(sigval_t a_val);

    TCallback m_handler;

    c::milliseconds m_interval;
    bool m_recurring;

    timer_t m_timer;
};

Timer::Impl::Impl()
    : m_recurring(false)
{
    sigevent sigev;
    memset(&sigev, 0, sizeof(sigev));

    sigev.sigev_value.sival_ptr = this;
    sigev.sigev_notify = SIGEV_THREAD;
    sigev.sigev_notify_attributes = NULL;
    sigev.sigev_notify_function = &Impl::p_StaticHandler;

    if (timer_create(CLOCK_REALTIME, &sigev, &m_timer) != 0)
    {
        throw runtime_error("Failed to create the timer");
    }
}

Timer::Impl::Impl(c::milliseconds a_interval, bool a_recurring)
    : Impl()
{
    SetInterval(a_interval, a_recurring);
}

Timer::Impl::~Impl()
{
    timer_delete(m_timer);
}

void Timer::Impl::SetInterval(c::milliseconds a_interval,
                              bool a_recurring)
{
    m_interval = a_interval;
    m_recurring = a_recurring;
}

void Timer::Impl::SetCallback(TCallback a_callback)
{
    m_handler = move(a_callback);
}

void Timer::Impl::Start()
{
    uint64_t l_nSec = m_interval.count() / 1000;
    uint64_t l_nNano =
            c::duration_cast<c::nanoseconds>(
                    c::milliseconds(m_interval.count() - (l_nSec * 1000))
            ).count();

    itimerspec l_new;
    memset(&l_new, 0, sizeof(l_new));

    l_new.it_value.tv_sec = l_nSec;
    l_new.it_value.tv_nsec = l_nNano;

    if (m_recurring)
    {
        l_new.it_interval.tv_sec = l_nSec;
        l_new.it_interval.tv_nsec = l_nNano;
    }

    if (timer_settime(m_timer, 0, &l_new, nullptr) != 0)
    {
        throw runtime_error("Failed to start the timer.");
    }
}

void Timer::Impl::Stop()
{
    itimerspec l_stopper;
    memset(&l_stopper, 0, sizeof(l_stopper));

    if (timer_settime(m_timer, 0, &l_stopper, nullptr) != 0)
    {
        throw runtime_error("Failed to stop the timer.");
    }
}

void Timer::Impl::p_Handler()
{
    if (m_handler)
    {
        m_handler();
    }
}





void Timer::Impl::p_StaticHandler(sigval_t a_val)
{
    Impl *l_obj = (Impl*)a_val.sival_ptr;
    l_obj->p_Handler();
}

}
}


