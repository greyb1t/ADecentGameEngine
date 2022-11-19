#pragma once
#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h" 

class GraphNode_SetGameObjectLocalPosition : public GraphNodeBase
{
public:
	GraphNode_SetGameObjectLocalPosition();

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	virtual std::string GetNodeName() const override { return "Set GameObject Local Position"; }
	virtual std::string GetNodeTypeCategory() const override { return "Game Object"; }
};

