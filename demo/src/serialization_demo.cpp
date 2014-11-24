/*
 * serialization_demo.cpp
 *
 *  Created on: Nov 10, 2014
 *      Author: mike
 */

#include <iostream>
#include <vector>
#include <memory>
#include <assert.h>

#include "serialization/master.h"

using namespace std;
namespace ser = axon::serialization;

bool s_fullTest = false;

class Node
{
public:
	// This could be a unique_ptr as well, which would be better design,
	// but initializer lists don't allow move semantics, so that would make this
	// demo far more verbose
	typedef shared_ptr<Node> Ptr;

	int Value;
	float Val2;
	double Val3;
	bool V4;
	vector<Ptr> Children;

	Node() : Value(0), Val2(-5), Val3(10003440599.2343), V4(false) { }
	Node(int val, float val2, double val3)
	    : Value(val), Val2(val2), Val3(val3), V4(val % 2 == 1) { }
	Node(int val, float val2, double val3, vector<Ptr> children)
		: Value(val), Val2(val2), Val3(val3), V4(val % 2 == 1), Children(move(children)) { }
	template<typename ...Nodes>
	Node(int val, float val2, double val3, Nodes ...children)
		: Value(val), Val2(val2), Val3(val3), V4(val % 2 == 1)
	{
		AddChildren(move(children)...);
	}

	static Ptr Create(int val, float val2, double val3)
	{
		return Ptr(new Node(val, val2, val3));
	}
	static Ptr Create(int val, float val2, double val3, vector<Ptr> children)
	{
		return Ptr(new Node(val, val2, val3, move(children)));
	}
	template<typename ...Nodes>
	static Ptr Create(int val, float val2, double val3, Nodes ...children)
	{
		return Ptr(new Node(val, val2, val3, move(children)...));
	}

	bool operator==(const Node &other) const
	{
		if (Value != other.Value)
			return false;

		// This is a lazy way to get around the fact that the xml and json
		// serializers don't necessarily perfectly encode floating point values
		if (s_fullTest)
		{
            if (Val2 != other.Val2)
                return false;
            if (Val3 != other.Val3)
                return false;
		}

		if (V4 != other.V4)
            return false;

		if (Children.size() != other.Children.size())
			return false;

		for (size_t i = 0; i < Children.size(); ++i)
		{
			if ((Children[i].get() == nullptr && other.Children[i].get() != nullptr) ||
			    (Children[i].get() != nullptr && other.Children[i].get() == nullptr))
				return false;

			if (*Children[i] != *other.Children[i])
				return false;
		}

		return true;
	}
	bool operator!=(const Node &other) const
	{
		return not (*this == other);
	}

private:
	void AddChildren() { }
	template<typename Curr, typename ...Nodes>
	void AddChildren(Curr c, Nodes ...children)
	{
		Children.push_back(move(c));
		AddChildren(move(children)...);
	}
};

// This is the magic function that tells Axon how to properly serialize and deserialize
// your object. The benefit of it being a free function is that you can implement this in
// a completely separate header, allowing you to separate your business objects from their
// serialization
void BindStruct(const ser::CStructBinder &binder, Node &node)
{
	binder("Value", node.Value)
	      ("Val2", node.Val2)
	      ("Val3", node.Val3)
	      ("V4", node.V4)
	      ("Children", node.Children);
}

class SomeType
{
public:
	typedef shared_ptr<SomeType> Ptr;

	SomeType(string name = "")
		: m_name(move(name)), m_nodeCt(0) { }
	SomeType(string name, vector<Node::Ptr> nodes)
		: m_name(move(name)), m_nodes(move(nodes))
	{
		m_nodeCt = m_nodes.size();
	}

	const string &GetName() const { return m_name; }
	void SetName(string a_name) { m_name = move(a_name); }

	const vector<Node::Ptr> &GetNodes() const { return m_nodes; }

	void SetNodes(vector<Node::Ptr> a_nodes)
	{
	    m_nodes = move(a_nodes);

	    m_nodeCt = m_nodes.size();
	}

	bool operator==(const SomeType &other) const
	{
		if (m_name != other.m_name)
			return false;
		if (m_nodeCt != other.m_nodeCt)
			return false;

		if (m_nodes.size() != other.m_nodes.size())
			return false;

		for (size_t i = 0; i < m_nodes.size(); ++i)
		{
			if ((m_nodes[i].get() == nullptr && other.m_nodes[i].get() != nullptr) ||
			    (m_nodes[i].get() != nullptr && other.m_nodes[i].get() == nullptr))
				return false;

			if (*m_nodes[i] != *other.m_nodes[i])
				return false;
		}

		return true;
	}

private:
	string m_name;
	vector<Node::Ptr> m_nodes;
	size_t m_nodeCt;
};

// If you need to do something differently when writing out an object or reading it back in,
// then you can implement two functions instead of the single bind function, which enables
// you to customize logic for the read and write cases
void BindStruct(const ser::CStructBinder &a_binder, SomeType &a_val)
{
    a_binder("Name", a_val, &SomeType::GetName, &SomeType::SetName)
            ("Nodes", a_val, &SomeType::GetNodes, &SomeType::SetNodes);
}

int main(int argc, char *argv[])
{
	SomeType st{
		"Hello World",
		{
			Node::Create(1, -1, 12345.678),
			Node::Create(2, 22, 2222),
			Node::Create(-1, 123, 456,
					Node::Create(4, 654.321, 11992200.336644),
					Node::Create(5, 123, 456.789,
							Node::Create(6, 7, 8)),
					Node::Create(7, -500000, -12345664.342354)),
			Node::Create(123456789, 21320943, 12432.34254)
		}
	};

	// Serialize the object to JSON
	cout << "JSON Serialization:" << endl << endl
		 << ser::CJsonSerializer().Serialize(st) << endl << endl << endl;

	// Serialize the same object to XML
	cout << "XML Serialization:" << endl << endl
	     << ser::CXmlSerializer().Serialize(st) << endl;

	// Serialize the object, then deserialize it, then verify that
	// the reconstruction was proper
	string jsonSerialized = ser::CJsonSerializer().Serialize(st);

	auto dsJson = ser::CJsonSerializer().Deserialize<SomeType>(jsonSerialized);

	assert(st == dsJson);

	// Convert the JSON serialization to XML without an intermediate type
	ser::AData::Ptr opaque = ser::CJsonSerializer().Deserialize(jsonSerialized);

	string xmlSerialized = ser::CXmlSerializer().SerializeData(*opaque);

	auto dsXml = ser::CXmlSerializer().Deserialize<SomeType>(xmlSerialized);

	assert(dsXml == st && dsXml == dsJson);

	s_fullTest = true;

	// Play with the MsgPack format. This is a standardized format.
	// http://msgpack.org/
	string msgPackSerialized = ser::CMsgPackSerializer().Serialize(st);

	auto dsMsgPack = ser::CMsgPackSerializer().Deserialize<SomeType>(msgPackSerialized);

	assert(dsMsgPack == st);

	// Play with the Axon binary format
	string axonSerialized = ser::CAxonSerializer().Serialize(st);

	auto dsAxon = ser::CAxonSerializer().Deserialize<SomeType>(axonSerialized);

	assert(dsAxon == st);

	cout << "JSON Format Length: " << jsonSerialized.size() << endl
	     << "XML Format Length: " << xmlSerialized.size() << endl
	     << "MsgPack Format Length: " << msgPackSerialized.size() << endl
	     << "Axon Format Length: " << axonSerialized.size() << endl;

	return EXIT_SUCCESS;
}



































