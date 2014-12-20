/*
 * disconnected_exception.h
 *
 *  Created on: Dec 17, 2014
 *      Author: mranzinger
 */

#pragma once

#include <stdexcept>

#include "dll_export.h"

namespace axon { namespace communication {

class AXON_COMMUNICATE_API CDisconnectedException
    : public std::runtime_error
{
public:
    CDisconnectedException()
        : std::runtime_error("The client is disconnected.")
    {

    }
};

} }


