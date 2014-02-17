/*
 * File description: demo.cpp
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#include <iostream>
#include <assert.h>
#include <random>

#include "serialization/master.h"

#include "communication/messaging/axon_client.h"
#include "communication/tcp/tcp_data_connection.h"

using namespace std;
using namespace axon::util;
using namespace axon::serialization;
using namespace axon::communication;
using namespace axon::communication::tcp;

int main(int argc, char *argv[])
{
	CContract<int (int, int)> l_add("Add");

	string l_hostName = "127.0.0.1";
	int l_port = 12345;

	cout << "Connecting to server " << l_hostName << ":" << l_port << endl;

	auto l_client = CAxonClient::Create(make_shared<CTcpDataConnection>(l_hostName, l_port));

	cout << "Connected!" << endl;

	srand(42);

	int l_sum = 0;
	for (size_t i = 0; i < 10000000; ++i)
	{
		if ((i % 10000) == 0)
		{
			cout << "Iteration " << i << endl;
		}

		l_sum += l_client->Send(l_add, rand(), rand());
	}

	cout << "Sum of randoms: " << l_sum << endl;

	return 0;
}























