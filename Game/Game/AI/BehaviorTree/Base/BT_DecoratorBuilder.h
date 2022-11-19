#pragma once
#include "BT_Decorator.h"

template <class Parent>
class BT_CompositeBuilder;

template <class Parent>
class BT_DecoratorBuilder
{
public:
	BT_DecoratorBuilder(Parent* parent, BT_Decorator* node) : parent(parent), node(node) {}

	template <class NodeType, typename... Args>
	BT_DecoratorBuilder<Parent> leaf(Args... args)
	{
		auto child = std::make_shared<NodeType>((args)...);
		node->setChild(child);
		return *this;
	}

	template <class CompositeType, typename... Args>
	BT_CompositeBuilder<BT_DecoratorBuilder<Parent>> composite(Args... args)
	{
		auto child = std::make_shared<CompositeType>((args)...);
		node->setChild(child);
		return BT_CompositeBuilder<BT_DecoratorBuilder<Parent>>(this, (CompositeType*)child.get());
	}

	template <class DecoratorType, typename... Args>
	BT_DecoratorBuilder<BT_DecoratorBuilder<Parent>> decorator(Args... args)
	{
		auto child = std::make_shared<DecoratorType>((args)...);
		node->setChild(child);
		return BT_DecoratorBuilder<BT_DecoratorBuilder<Parent>>(this, (DecoratorType*)child.get());
	}

	Parent& end()
	{
		return *parent;
	}

private:
	Parent* parent;
	BT_Decorator* node;
};

