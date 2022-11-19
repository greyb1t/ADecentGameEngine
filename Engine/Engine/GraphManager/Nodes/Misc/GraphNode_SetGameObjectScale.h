#pragma once
#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h" 

class GraphNode_SetGameObjectScale : public GraphNodeBase
{
public:
	GraphNode_SetGameObjectScale();

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	virtual std::string GetNodeName() const override { return "Set GameObject Scale"; }
	virtual std::string GetNodeTypeCategory() const override { return "Game Object"; }
};

