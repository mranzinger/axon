/*
 * File description: demo.cpp
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#include <iostream>
#include <assert.h>

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

	cout << "Adding 1 and 2" << endl;

	int l_val = l_client->Send(l_add, 1, 2);

	cout << "Result: " << l_val << endl;

	return 0;
}























