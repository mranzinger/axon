/*
 * File description: demo.cpp
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#include <iostream>
#include <assert.h>

#include "serialization/master.h"

#include "communication/messaging/axon_client.h"

using namespace std;
using namespace axon::util;
using namespace axon::serialization;
using namespace axon::communication;

int main(int argc, char *argv[])
{
	CContract<int (int, int)> l_add("Add");

	auto l_client = CAxonClient::Create();

	int l_val = l_client->Send(l_add, 1, 2);

	return 0;
}























