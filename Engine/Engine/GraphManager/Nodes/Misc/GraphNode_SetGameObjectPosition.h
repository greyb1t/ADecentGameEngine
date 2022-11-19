#pragma once
#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h" 

class GraphNode_SetGameObjectPosition : public GraphNodeBase
{
public:
	GraphNode_SetGameObjectPosition();

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	virtual std::string GetNodeName() const override { return "Set GameObject Position"; }
	virtual std::string GetNodeTypeCategory() const override { return "Game Object"; }
};

