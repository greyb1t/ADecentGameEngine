#pragma once

#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h"

class GraphNode_FindGameObjectID : public GraphNodeBase
{
public:
	GraphNode_FindGameObjectID();

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	virtual std::string GetNodeName() const override { return "Find Game Object ID"; }
	virtual std::string GetNodeTypeCategory() const override { return "Game Object"; }
};