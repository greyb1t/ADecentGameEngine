#pragma once

#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h"

class GraphNode_MathMin : public GraphNodeBase
{
public:
	GraphNode_MathMin();

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	virtual std::string GetNodeName() const override { return "Min"; }
	virtual std::string GetNodeTypeCategory() const override { return "Math"; }
};