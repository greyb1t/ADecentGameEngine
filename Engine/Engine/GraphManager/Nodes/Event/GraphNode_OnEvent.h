#pragma once

#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h"
#include "NodeEventStructs.h"

class GraphNode_OnEvent : public GraphNodeBase
{
public:
	GraphNode_OnEvent(const std::string& aEventName, const std::vector<DynamicPin>& aDynamicPins);

	int OnExecWithPayload(GraphNodeInstance* aNodeInstance, Payload& aPayload) override;

	virtual std::string GetNodeName() const override { return "On " + myEventName; }

	std::string GetNodeTypeCategory() const override { return "Events"; }

	FORCEINLINE bool IsStartNode() const override { return true; }

private:
	std::string myEventName;
	std::vector<DynamicPin> myDynamicPins;
};