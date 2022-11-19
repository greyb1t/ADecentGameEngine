#pragma once

#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h"

class GraphNode_MathAtan2 : public GraphNodeBase
{
public:
	GraphNode_MathAtan2();

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	virtual std::string GetNodeName() const override { return "Atan2"; }
	virtual std::string GetNodeTypeCategory() const override { return "Math"; }
};