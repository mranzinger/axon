/*
 * File description: server_demo.cpp
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#include <iostream>
#include <assert.h>
#include <atomic>
#include <chrono>

#include "serialization/master.h"

#include "communication/messaging/axon_server.h"
#include "communication/tcp/tcp_data_server.h"

using namespace std;
using namespace std::chrono;
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

	atomic<uint32_t> l_hitCt{0};

	auto l_start = steady_clock::now();

	auto l_server = CAxonServer::Create(make_shared<CTcpDataServer>(l_port));

	auto lb = [&] (int a, int b) -> int
			{
				++l_hitCt;

				if ((l_hitCt % 100000) == 0)
				{
					auto l_end = steady_clock::now();

					cout << "Processed the last 100,000 calls in "
						 << duration_cast<milliseconds>(l_end - l_start).count()
						 << "ms." << endl;

					l_start = l_end;
				}

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
