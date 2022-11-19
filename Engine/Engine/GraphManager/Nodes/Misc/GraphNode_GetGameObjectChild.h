#pragma once
#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h" 

class GraphNode_GetGameObjectChild : public GraphNodeBase
{
public:
	GraphNode_GetGameObjectChild();

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	virtual std::string GetNodeName() const override { return "Get GameObject Child"; }
	virtual std::string GetNodeTypeCategory() const override { return "Game Object"; }
};

