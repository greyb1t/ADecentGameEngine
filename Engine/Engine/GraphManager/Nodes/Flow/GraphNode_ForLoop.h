#pragma once

#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h"

class GraphNode_ForLoop : public GraphNodeBase
{
public:
	GraphNode_ForLoop();

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	virtual std::string GetNodeName() const override { return "ForLoop"; }

	std::string GetNodeTypeCategory() const override { return "Flow"; }
};