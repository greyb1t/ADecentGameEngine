#pragma once
#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h" 

class GraphNode_EqualsInt : public GraphNodeBase
{
public:
	GraphNode_EqualsInt();

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	virtual std::string GetNodeName() const override { return "Equals Int"; }
	virtual std::string GetNodeTypeCategory() const override { return "Misc"; }
};

