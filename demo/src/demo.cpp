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
using namespace axon::serialization;
using namespace axon::communication;

int main(int argc, char *argv[])
{
	//map<int, string> l_map1{ { 0, "foo" }, { 1, "bar" }, { 2, "baz" } };

	//vector<double> l_vec(20000);

	//test(CJsonSerializer(), l_vec);
	//test(CXmlSerializer(), l_vec);
	//test(CAxonSerializer(), l_vec);

	//test(CAxonSerializer(), shared_ptr<CBase>(new CDerived));

	CContract<void (int, int)> l_add("Add");

	CMessage::Ptr l_msg = l_add.Serialize(42, 45);

	auto l_tuple = l_add.DeserializeArgs(*l_msg);

	return 0;
}























