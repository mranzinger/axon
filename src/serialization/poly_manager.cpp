/*
 * File description: poly_manager.cpp
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#include "serialization/base/poly_manager.h"

namespace axon { namespace serialization {

static CPolyManager *s_instance = nullptr;

CPolyManager::CPolyManager()
{

}

CPolyManager* CPolyManager::p_GetInstance()
{
	if (!s_instance)
		s_instance = new CPolyManager;

	return s_instance;
}

}
}


