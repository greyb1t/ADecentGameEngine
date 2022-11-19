#pragma once

#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h"

class GraphNode_Branch : public GraphNodeBase
{
public:
	GraphNode_Branch();

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	virtual std::string GetNodeName() const override { return "Branch"; }

	std::string GetNodeTypeCategory() const override { return "Flow"; }
};