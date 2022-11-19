#pragma once
#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h" 

class GraphNode_EqualsString : public GraphNodeBase
{
public:
	GraphNode_EqualsString();

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	virtual std::string GetNodeName() const override { return "Equals String"; }
	virtual std::string GetNodeTypeCategory() const override { return "Misc"; }
};

