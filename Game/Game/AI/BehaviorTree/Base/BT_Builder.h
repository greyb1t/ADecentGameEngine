#pragma once
#include "BT_BehaviorTree.h"
#include "BT_CompositeBuilder.h"
#include "BT_DecoratorBuilder.h"
#include "BT_Node.h"

class BT_Builder
{
public:
	template <class NodeType, typename... Args>
	BT_Builder leaf(Args... args)
	{
		root = std::make_shared<NodeType>((args)...);
		return *this;
	}

	template <class CompositeType, typename... Args>
	BT_CompositeBuilder<BT_Builder> composite(Args... args)
	{
		root = std::make_shared<CompositeType>((args)...);
		return BT_CompositeBuilder<BT_Builder>(this, (CompositeType*)root.get());
	}

	template <class DecoratorType, typename... Args>
	BT_DecoratorBuilder<BT_Builder> decorator(Args... args)
	{
		root = std::make_shared<DecoratorType>((args)...);
		return BT_DecoratorBuilder<BT_Builder>(this, (DecoratorType*)root.get());
	}

	BT_Node::Ptr build();

private:
	BT_Node::Ptr root;
};

