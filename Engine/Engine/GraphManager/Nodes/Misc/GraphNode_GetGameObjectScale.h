#pragma once
#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h" 

class GraphNode_GetGameObjectScale : public GraphNodeBase
{
public:
	GraphNode_GetGameObjectScale();

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	virtual std::string GetNodeName() const override { return "Get GameObject Scale"; }
	virtual std::string GetNodeTypeCategory() const override { return "Game Object"; }
};

