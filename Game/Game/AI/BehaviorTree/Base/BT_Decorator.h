#pragma once
#include "BT_Node.h"

class BT_Decorator : public BT_Node
{
public:
	virtual ~BT_Decorator() {}

	void setChild(BT_Node::Ptr node) { child = node; }
	bool hasChild() const { return child != nullptr; }

protected:
	BT_Node::Ptr child = nullptr;
};
