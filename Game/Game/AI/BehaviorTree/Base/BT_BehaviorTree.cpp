#include "pch.h"
#include "BT_BehaviorTree.h"

BT_Node::Status BT_BehaviorTree::update()
{
	return root->tick();
}

void BT_BehaviorTree::setRoot(const BT_Node::Ptr& node)
{
	root = node;
}

BT_Blackboard::Ptr BT_BehaviorTree::getBlackboard() const
{
	return blackboard;
}
