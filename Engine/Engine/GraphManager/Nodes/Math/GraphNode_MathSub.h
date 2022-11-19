#pragma once

#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h"

class GraphNode_MathSub : public GraphNodeBase
{
public:
	GraphNode_MathSub();

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	virtual std::string GetNodeName() const override { return "Sub"; }
	virtual std::string GetNodeTypeCategory() const override { return "Math"; }
};