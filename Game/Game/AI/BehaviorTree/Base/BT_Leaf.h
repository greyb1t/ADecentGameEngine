#pragma once
#include "BT_Blackboard.h"
#include "BT_Node.h"

class BT_Leaf : public BT_Node
{
public:
	BT_Leaf() {}
	virtual ~BT_Leaf() {}
	BT_Leaf(BT_Blackboard::Ptr aBlackboard) : myBlackboard(aBlackboard) {}

	virtual Status update() = 0;

protected:
	BT_Blackboard::Ptr myBlackboard;
};
