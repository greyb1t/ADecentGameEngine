#pragma once
#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h" 

class GraphNode_EqualsGameObject : public GraphNodeBase
{
public:
	GraphNode_EqualsGameObject();

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	virtual std::string GetNodeName() const override { return "Equals GameObject"; }
	virtual std::string GetNodeTypeCategory() const override { return "Misc"; }
};

