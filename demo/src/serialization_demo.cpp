/*
 * serialization_demo.cpp
 *
 *  Created on: Nov 10, 2014
 *      Author: mike
 */

#include <iostream>
#include <vector>
#include <memory>

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

	float Value;
	vector<Ptr> Children;

	Node(float val = 0.0f) : Value(val) { }
	Node(float val, vector<Ptr> children)
		: Value(val), Children(move(children)) { }
	template<typename ...Nodes>
	Node(float val, Nodes ...children)
		: Value(val)
	{
		AddChildren(move(children)...);
	}

	static Ptr Create(float val = 0.0f)
	{
		return Ptr(new Node(val));
	}
	static Ptr Create(float val, vector<Ptr> children)
	{
		return Ptr(new Node(val, move(children)));
	}
	template<typename ...Nodes>
	static Ptr Create(float val, Nodes ...children)
	{
		return Ptr(new Node(val, move(children)...));
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
		: m_name(move(name)) { }
	SomeType(string name, vector<Node::Ptr> nodes)
		: m_name(move(name)), m_nodes(move(nodes)) { }

	void Bind(const ser::CStructBinder &binder)
	{
		binder("Name", m_name)
		      ("Nodes", m_nodes);
	}

private:
	string m_name;
	vector<Node::Ptr> m_nodes;
};

void BindStruct(const ser::CStructBinder &binder, SomeType &st)
{
	st.Bind(binder);
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

	cout << "JSON Serialization:" << endl << endl
		 << ser::CJsonSerializer().Serialize(st) << endl << endl << endl;

	cout << "XML Serialization:" << endl << endl
	     << ser::CXmlSerializer().Serialize(st) << endl;

	return EXIT_SUCCESS;
}



































