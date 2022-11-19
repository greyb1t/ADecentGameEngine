#pragma once
#include "BT_Blackboard.h"
#include "BT_Node.h"

class BT_BehaviorTree : public BT_Node
{
public:
	BT_BehaviorTree() : blackboard(DBG_NEW BT_Blackboard()) {}
	BT_BehaviorTree(const BT_Node::Ptr& rootNode) : BT_BehaviorTree() { root = rootNode; }

	Status update();
	

	void setRoot(const BT_Node::Ptr& node);
	BT_Blackboard::Ptr getBlackboard() const;

private:
	BT_Node::Ptr root = nullptr;
	BT_Blackboard::Ptr blackboard = nullptr;
};

