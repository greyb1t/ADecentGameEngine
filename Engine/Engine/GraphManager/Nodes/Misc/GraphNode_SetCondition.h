#pragma once
#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h" 

class GraphNode_SetCondition : public GraphNodeBase
{
public:
	GraphNode_SetCondition();

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	virtual std::string GetNodeName() const override { return "Set Condition"; }
	virtual std::string GetNodeTypeCategory() const override { return "Animation"; }
};

