#pragma once

#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h"
#include "NodeEventStructs.h"

class GraphNode_EventCall : public GraphNodeBase
{
public:
	GraphNode_EventCall(const std::string& aEventName, const std::vector<DynamicPin>& aDynamicPins);

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	virtual std::string GetNodeName() const override { return "Call " + myEventName; }

	std::string GetNodeTypeCategory() const override { return "Events"; }

private:
	std::string myEventName;
	std::vector<DynamicPin> myDynamicPins;

};