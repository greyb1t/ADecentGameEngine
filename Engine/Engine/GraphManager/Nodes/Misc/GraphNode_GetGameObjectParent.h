#pragma once
#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h" 

class GraphNode_GetGameObjectParent : public GraphNodeBase
{
public:
	GraphNode_GetGameObjectParent();

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	virtual std::string GetNodeName() const override { return "Get GameObject Parent"; }
	virtual std::string GetNodeTypeCategory() const override { return "Game Object"; }
};

