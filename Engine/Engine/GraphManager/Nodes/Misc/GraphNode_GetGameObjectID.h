#pragma once

#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h"

class GraphNode_GetGameObjectID : public GraphNodeBase
{
public:
	GraphNode_GetGameObjectID();

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	virtual std::string GetNodeName() const override { return "Get Game Object ID"; }
	virtual std::string GetNodeTypeCategory() const override { return "Game Object"; }
};