#pragma once
#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h" 

class GraphNode_SetWindowedMode : public GraphNodeBase
{
public:
	GraphNode_SetWindowedMode();

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	virtual std::string GetNodeName() const override { return "Set Windowed Mode"; }
	virtual std::string GetNodeTypeCategory() const override { return "Settings"; }
};

