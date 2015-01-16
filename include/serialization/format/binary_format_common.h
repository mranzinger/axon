/*
 * binary_format_common.h
 *
 *  Created on: Jan 16, 2015
 *      Author: mranzinger
 */

#pragma once

#include <stdexcept>
#include <exception>

#include "dll_export.h"

namespace axon { namespace serialization {

class CBufferOverflowException
    : public std::exception
{
public:
    virtual const char *what() const noexcept override
    {
        return "The specified binary operation failed because it would result in an overflow.";
    }
};

} }


