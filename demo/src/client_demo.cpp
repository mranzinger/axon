/*
 * File description: demo.cpp
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#include <iostream>
#include <assert.h>
#include <random>
#include <unistd.h>

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

	int l_sleepTime = 0;
	if (argc == 2)
		l_sleepTime = atoi(argv[1]);

	cout << "Sleep Interval: " << l_sleepTime << endl;

	int l_toCt = 0;

	int l_sum = 0;
	for (size_t i = 0; i < 10000000; ++i)
	{
		if ((i % 10000) == 0)
		{
			cout << "Iteration " << i << endl;
		}

		try
		{
			l_sum += l_client->Send(l_add, 100, rand(), rand());
		}
		catch (CTimeoutException &ex)
		{
			++l_toCt;
			cout << "Timeout Occurred. Count: " << l_toCt << endl;
		}

		if (l_sleepTime)
		{
			usleep(l_sleepTime * 1000);
		}
	}

	cout << "Sum of randoms: " << l_sum << endl;
	cout << "Num Timeouts: " << l_toCt << endl;

	return 0;
}























