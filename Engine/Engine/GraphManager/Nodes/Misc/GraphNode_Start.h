#pragma once
#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h"
class GraphNode_Start : public GraphNodeBase
{
public:
	GraphNode_Start();

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	bool IsStartNode() const override { return true; }

	virtual std::string GetNodeName() const override { return "Start"; }
private:
	
};

