#include "pch.h"
#include "BT_Builder.h"

BT_Node::Ptr BT_Builder::build()
{
	assert(root != nullptr && "The Behavior Tree is empty!");
	auto tree = std::make_shared<BT_BehaviorTree>();
	tree->setRoot(root);
	return tree;
}