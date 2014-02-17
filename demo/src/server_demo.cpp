/*
 * File description: server_demo.cpp
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#include <iostream>
#include <assert.h>

#include "serialization/master.h"

#include "communication/messaging/axon_server.h"
#include "communication/tcp/tcp_data_server.h"

using namespace std;
using namespace axon::util;
using namespace axon::serialization;
using namespace axon::communication;
using namespace axon::communication::tcp;

int main(int argc, char *argv[])
{
	CContract<int (int, int)> l_add("Add");

	int l_port = 12345;

	cout << "Running server on port " << l_port << endl;
	cout << "Press q+[Enter] to terminate..." << endl;

	auto l_server = CAxonServer::Create(make_shared<CTcpDataServer>(l_port));

	auto lb = [] (int a, int b) -> int
			{
				//cout << a << " + " << b << " = " << (a + b) << endl;

				return a + b;
			};

	l_server->HostContract(l_add, lb);

	//while (true)
	//{
		string s;
		getline(cin,s);

		//if (s == "q")
		//	break;
	//}

	cout << "Terminating Server..." << endl;
}
