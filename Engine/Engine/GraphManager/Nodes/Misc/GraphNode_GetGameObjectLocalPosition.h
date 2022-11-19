#pragma once
#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h" 

class GraphNode_GetGameObjectLocalPosition : public GraphNodeBase
{
public:
	GraphNode_GetGameObjectLocalPosition();

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	virtual std::string GetNodeName() const override { return "Get GameObject Local Position"; }
	virtual std::string GetNodeTypeCategory() const override { return "Game Object"; }
};

