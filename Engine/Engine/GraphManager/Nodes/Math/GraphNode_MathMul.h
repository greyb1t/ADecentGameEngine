#pragma once

#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h"

class GraphNode_MathMul : public GraphNodeBase
{
public:
	GraphNode_MathMul();

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	virtual std::string GetNodeName() const override { return "Mul"; }
	virtual std::string GetNodeTypeCategory() const override { return "Math"; }
};