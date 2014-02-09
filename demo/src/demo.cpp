/*
 * File description: demo.cpp
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#include <iostream>
#include <assert.h>

#include "serialization/master.h"

using namespace std;
using namespace axon::serialization;


struct CBase
{
	virtual ~CBase() { }

	int a = 42;
};

void BindStruct(const CStructBinder &a_binder, CBase &a_val)
{
	a_binder("a", a_val.a);
}

AXON_SERIALIZE_BASE_TYPE(CBase);

struct CDerived
	: CBase
{
	std::string b = "Hello World";
};

AXON_SERIALIZE_DERIVED_TYPE(CBase, CDerived, "Derived");

void BindStruct(const CStructBinder &a_binder, CDerived &a_val)
{
	BindStruct(a_binder, (CBase&)a_val);

	a_binder("b", a_val.b);
}

template<typename T>
void test(const ASerializer &a_ser, const T &a_val)
{
	string l_enc = a_ser.Serialize(a_val);

	cout << "Format: " << a_ser.FormatName() << endl;
	cout << "Encoded Size: " << l_enc.size() << endl;

	if (l_enc.size() < 300)
		cout << l_enc << endl;

	T l_tmp;

	a_ser.Deserialize(l_enc, l_tmp);

	string l_enc2 = a_ser.Serialize(l_tmp);

	cout << "Are Same? " << boolalpha << (l_enc == l_enc2) << endl << endl;
}

int main(int argc, char *argv[])
{
	cout << sizeof(vector<double>) << endl;

	//map<int, string> l_map1{ { 0, "foo" }, { 1, "bar" }, { 2, "baz" } };

	vector<double> l_vec(20000);

	//test(CJsonSerializer(), l_vec);
	//test(CXmlSerializer(), l_vec);
	test(CAxonSerializer(), l_vec);

	test(CAxonSerializer(), shared_ptr<CBase>(new CDerived));

	return 0;
}


