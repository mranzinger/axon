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

class Node
{
public:
	// This could be a unique_ptr as well, which would be better design,
	// but initializer lists don't allow move semantics, so that would make this
	// demo far more verbose
	typedef shared_ptr<Node> Ptr;

	int Value;
	vector<Ptr> Children;

	Node(int val = 0) : Value(val) { }
	Node(int val, vector<Ptr> children)
		: Value(val), Children(move(children)) { }
	template<typename ...Nodes>
	Node(int val, Nodes ...children)
		: Value(val)
	{
		AddChildren(move(children)...);
	}

	static Ptr Create(int val = 0)
	{
		return Ptr(new Node(val));
	}
	static Ptr Create(int val, vector<Ptr> children)
	{
		return Ptr(new Node(val, move(children)));
	}
	template<typename ...Nodes>
	static Ptr Create(int val, Nodes ...children)
	{
		return Ptr(new Node(val, move(children)...));
	}

	bool operator==(const Node &other) const
	{
		if (Value != other.Value)
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

	void Write(const ser::CStructWriter &writer) const
	{
		writer("Name", m_name)
			  ("Nodes", m_nodes);
	}
	void Read(const ser::CStructReader &reader)
	{
		reader("Name", m_name)
			  ("Nodes", m_nodes);
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
void WriteStruct(const ser::CStructWriter &writer, const SomeType &st)
{
	st.Write(writer);
}

void ReadStruct(const ser::CStructReader &reader, SomeType &st)
{
	st.Read(reader);
}

int main(int argc, char *argv[])
{
	SomeType st{
		"Hello World",
		{
			Node::Create(1),
			Node::Create(2),
			Node::Create(3,
					Node::Create(4),
					Node::Create(5,
							Node::Create(6)),
					Node::Create(7)),
			Node::Create(8)
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

	return EXIT_SUCCESS;
}



































