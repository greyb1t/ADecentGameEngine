#pragma once
#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h" 

class GraphNode_SetGameObjectTransform : public GraphNodeBase
{
public:
	GraphNode_SetGameObjectTransform();

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	virtual std::string GetNodeName() const override { return "Set GameObject Transform"; }
	virtual std::string GetNodeTypeCategory() const override { return "Game Object"; }
};

