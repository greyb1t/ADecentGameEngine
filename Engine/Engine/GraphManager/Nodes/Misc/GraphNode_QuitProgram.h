#pragma once
#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h" 

class GraphNode_QuitProgram : public GraphNodeBase
{
public:
	GraphNode_QuitProgram();

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	virtual std::string GetNodeName() const override { return "Quit Program"; }
	virtual std::string GetNodeTypeCategory() const override { return "Misc"; }
};

