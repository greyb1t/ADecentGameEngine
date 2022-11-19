#pragma once
#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h" 

class GraphNode_GetGameObjectTransform : public GraphNodeBase
{
public:
	GraphNode_GetGameObjectTransform();

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	virtual std::string GetNodeName() const override { return "Get GameObject Transform"; }
	virtual std::string GetNodeTypeCategory() const override { return "Game Object"; }
};

