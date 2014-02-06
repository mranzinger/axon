/*
 * File description: demo.cpp
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#include <iostream>
#include <assert.h>

#include "serialization/format/json_serializer.h"
#include "serialization/format/xml_serializer.h"
#include "serialization/format/axon_serializer.h"

using namespace std;
using namespace axon::serialization;

struct D
{
	float f;
	int *g = nullptr;
	std::vector<D*> h;
	std::map<int, D*> i;
	axon::util::CBuffer j;
};

template<typename Binder>
void BindStruct(Binder &a_binder, D &a_value)
{
	a_binder("f", a_value.f)("g", a_value.g)("h", a_value.h)("i", a_value.i)("j", a_value.j);
};

struct E
{
	string val;
};

ostream &operator<<(ostream &a, const E &e) { return a << e.val; }
istream &operator>>(istream &a, E&e) { return a >> e.val; }

struct some_struct
{
	int a, b;
	std::string c;
	D d;
	E e;
};

template<typename Binder>
void BindStruct(Binder &w, some_struct &s)
{
	w("a", s.a)("b", s.b)("c", s.c)("d", s.d)("e", s.e);
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
	//map<int, string> l_map1{ { 0, "foo" }, { 1, "bar" }, { 2, "baz" } };

	vector<double> l_vec(200000000);

	//test(CJsonSerializer(), l_vec);
	//test(CXmlSerializer(), l_vec);
	test(CAxonSerializer(), l_vec);

	//test(CJsonSerializer(), some_struct());
	//test(CXmlSerializer(), some_struct());
	//test(CAxonSerializer(), some_struct());

	return 0;
}


