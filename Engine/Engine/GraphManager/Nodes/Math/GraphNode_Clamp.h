#pragma once

#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h"

class GraphNode_MathClamp: public GraphNodeBase
{
public:
	GraphNode_MathClamp();

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	virtual std::string GetNodeName() const override { return "Clamp"; }
	virtual std::string GetNodeTypeCategory() const override { return "Math"; }
};