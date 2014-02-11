/*
 * File description: demo.cpp
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#include <iostream>
#include <assert.h>

#include "serialization/master.h"

#include "communication/messaging/contract.h"

using namespace std;
using namespace axon::util;
using namespace axon::serialization;
using namespace axon::communication;

int main(int argc, char *argv[])
{
	CContract<int (int, int)> l_add("Add");

	CMessage::Ptr l_msg = l_add.Serialize(42, 45);

	CMessage::Ptr l_ret = l_add.Invoke(*l_msg,
			[] (int a, int b)
			{
				return a + b;
			});

	auto l_result = l_add.DeserializeRet<int>(*l_ret);

	return 0;
}























