#pragma once
#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h" 

class GraphNode_GetGameObjectPosition : public GraphNodeBase
{
public:
	GraphNode_GetGameObjectPosition();

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	virtual std::string GetNodeName() const override { return "Get GameObject Position"; }
	virtual std::string GetNodeTypeCategory() const override { return "Game Object"; }
};

