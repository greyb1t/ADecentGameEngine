#pragma once
#include "BT_Composite.h"

template <class Parent>
class BT_DecoratorBuilder;

template <class Parent>
class BT_CompositeBuilder
{
public:
	BT_CompositeBuilder(Parent* parent, BT_Composite* node) : parent(parent), node(node) {}

	template <class NodeType, typename... Args>
	BT_CompositeBuilder<Parent> leaf(Args... args)
	{
		auto child = std::make_shared<NodeType>((args)...);
		node->addChild(child);
		return *this;
	}

	template <class CompositeType, typename... Args>
	BT_CompositeBuilder<BT_CompositeBuilder<Parent>> composite(Args... args)
	{
		auto child = std::make_shared<CompositeType>((args)...);
		node->addChild(child);
		return BT_CompositeBuilder<BT_CompositeBuilder<Parent>>(this, (CompositeType*)child.get());
	}

	template <class DecoratorType, typename... Args>
	BT_DecoratorBuilder<BT_CompositeBuilder<Parent>> decorator(Args... args)
	{
		auto child = std::make_shared<DecoratorType>((args)...);
		node->addChild(child);
		return BT_DecoratorBuilder<BT_CompositeBuilder<Parent>>(this, (DecoratorType*)child.get());
	}

	Parent& end()
	{
		return *parent;
	}

private:
	Parent* parent;
	BT_Composite* node;
};

