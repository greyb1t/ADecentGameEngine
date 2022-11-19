#pragma once

#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h"

class GraphNode_StopTimer : public GraphNodeBase
{
public:
	GraphNode_StopTimer();
	~GraphNode_StopTimer();

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	virtual std::string GetNodeName() const override { return "StopTimer"; }
};