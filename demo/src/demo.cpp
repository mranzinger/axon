/*
 * File description: demo.cpp
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#include <iostream>
#include <assert.h>

#include "serialization/format/json_serializer.h"

using namespace std;
using namespace axon::serialization;

struct D
{
	float f;
	int *g = nullptr;
	std::vector<D*> h;
	std::map<int, D*> i;
};

template<typename Binder>
void BindStruct(Binder &a_binder, D &a_value)
{
	a_binder("f", a_value.f)("g", a_value.g)("h", a_value.h)("i", a_value.i);
};

struct E { };

ostream &operator<<(ostream &a, const E &) { return a << "Foo"; }

struct some_struct
{
	int a, b;
	std::string c;
	D d;
	E e;
};

void WriteStruct(CStructWriter &w, const some_struct &s)
{
	w("a", s.a)("b", s.b)("c", s.c)("d", s.d)("e", s.e);
}

int main(int argc, char *argv[])
{
	map<int, string> l_map1{ { 0, "foo" }, { 1, "bar" }, { 2, "baz" } }, l_map2;

	string l_json = CJsonSerializer().Serialize(l_map1);

	cout << l_json << endl;

	CJsonSerializer().Deserialize(l_json, l_map2);

	cout << "Are Same? " << (l_map1 == l_map2) << endl;

	some_struct a, b;

	l_json = CJsonSerializer().Serialize(a);

	cout << endl << l_json << endl;

	return 0;
}


