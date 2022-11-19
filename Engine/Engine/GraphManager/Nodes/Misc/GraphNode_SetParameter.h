#pragma once
#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h" 

class GraphNode_SetParameter : public GraphNodeBase
{
public:
	GraphNode_SetParameter();

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	virtual std::string GetNodeName() const override { return "Set Audio Parameter"; }
	virtual std::string GetNodeTypeCategory() const override { return "Audio"; }
};

