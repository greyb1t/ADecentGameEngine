#pragma once

#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h"

class GraphNode_Timer : public GraphNodeBase
{
public:
	GraphNode_Timer();

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	void OnTimerElapsed(GraphNodeInstance* aNodeInstance);

	virtual std::string GetNodeName() const override { return "Timer"; }
};