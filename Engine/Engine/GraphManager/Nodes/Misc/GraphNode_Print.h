#pragma once
#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h"
#include "Engine/GraphManager/Nodes/Base/NodeTypes.h"

/**
 * Simple node that prints its value to the console.
 * Can be a String or a convertible data type such as float, int and bool.
 */
class GraphNode_Print : public GraphNodeBase
{
public:
	GraphNode_Print();

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	virtual std::string GetNodeName() const override { return "Print"; }
};

