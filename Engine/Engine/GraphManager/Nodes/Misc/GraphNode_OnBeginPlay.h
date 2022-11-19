#pragma once

#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h"

class GraphNode_OnBeginPlay : public GraphNodeBase
{
public:
	GraphNode_OnBeginPlay();

	int OnExecWithPayload(class GraphNodeInstance* aNodeInstance, Payload& aPayload) override;

	virtual std::string GetNodeName() const override { return "On Begin Play"; }

	FORCEINLINE bool IsStartNode() const override { return true; }

	FORCEINLINE std::string GetNodeTypeCategory() const override { return "Events"; }
};