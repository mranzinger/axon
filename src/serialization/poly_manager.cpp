/*
 * File description: poly_manager.cpp
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#include "base/poly_manager.h"

namespace axon { namespace serialization {

CPolyManager* CPolyManager::p_GetInstance()
{
	static CPolyManager s_instance;

	return &s_instance;
}

}
}


