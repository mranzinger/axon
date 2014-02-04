/*
 * File description: demo.cpp
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#include <iostream>
#include <assert.h>

#include "serialization/format/json_serializer.h"
#include "serialization/format/xml_serializer.h"

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

struct E { };

ostream &operator<<(ostream &a, const E &) { return a << "Foo"; }
istream &operator>>(istream &a, E&) { return a; }

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

	cout << l_enc << endl;

	T l_tmp;

	a_ser.Deserialize(l_enc, l_tmp);

	//cout << "Are Same? " << (a_val == l_tmp) << endl << endl;
}

int main(int argc, char *argv[])
{
	map<int, string> l_map1{ { 0, "foo" }, { 1, "bar" }, { 2, "baz" } };

	test(CJsonSerializer(), l_map1);
	test(CXmlSerializer(), l_map1);

	test(CJsonSerializer(), some_struct());
	test(CXmlSerializer(), some_struct());

	return 0;
}


