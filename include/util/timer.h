/*
 * timer.h
 *
 *  Created on: Dec 17, 2014
 *      Author: mranzinger
 */

#pragma once

#include <memory>
#include <chrono>
#include <functional>

#include "dll_export.h"

namespace axon { namespace util {

class AXON_UTIL_API Timer
{
    class Impl;

public:
    typedef std::function<void ()> TCallback;

    Timer();
    Timer(std::chrono::milliseconds a_interval,
          bool a_recurring = true);
    ~Timer();

    void SetInterval(std::chrono::milliseconds a_interval,
                     bool a_recurring = true);
    void SetCallback(TCallback a_handler);

    void Start();
    void Stop();

private:
    std::unique_ptr<Impl> m_impl;
};

} }
