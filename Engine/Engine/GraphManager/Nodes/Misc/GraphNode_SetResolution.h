#pragma once
#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h" 

class GraphNode_SetResolution : public GraphNodeBase
{
public:
	GraphNode_SetResolution();

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	virtual std::string GetNodeName() const override { return "Set Resolution"; }
	virtual std::string GetNodeTypeCategory() const override { return "Settings"; }
};

