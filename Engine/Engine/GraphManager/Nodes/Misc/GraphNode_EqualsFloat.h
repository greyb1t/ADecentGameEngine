#pragma once
#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h" 

class GraphNode_EqualsFloat : public GraphNodeBase
{
public:
	GraphNode_EqualsFloat();

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	virtual std::string GetNodeName() const override { return "Equals Float"; }
	virtual std::string GetNodeTypeCategory() const override { return "Misc"; }
};

