#pragma once
#include "BT_Node.h"

class BT_Composite : public BT_Node
{
public:
	BT_Composite() : it(children.begin()) {}
	virtual ~BT_Composite() {}

	void addChild(BT_Node::Ptr child) { children.push_back(child); }
	bool hasChildren() const { return !children.empty(); }

protected:
	std::vector<BT_Node::Ptr> children;
	std::vector<BT_Node::Ptr>::iterator it;
};