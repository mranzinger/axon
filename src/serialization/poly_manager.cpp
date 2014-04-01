/*
 * File description: poly_manager.cpp
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#include "serialization/base/poly_manager.h"

namespace axon { namespace serialization {

CPolyManager* CPolyManager::p_GetInstance()
{
	static CPolyManager s_instance;

	return &s_instance;
}

}
}


