#pragma once

#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h"

class GraphNode_MathLength : public GraphNodeBase
{
public:
	GraphNode_MathLength();

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	virtual std::string GetNodeName() const override { return "Length"; }
	virtual std::string GetNodeTypeCategory() const override { return "Math"; }
};