#pragma once

#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h"
#include "NodeEventStructs.h"
#include "Engine/UIEventManager/UIEventManager.h"

class GraphNode_OnTriggerEvent : public GraphNodeBase
{
public:
	GraphNode_OnTriggerEvent();

	int OnExecWithPayload(GraphNodeInstance* aNodeInstance, Payload& aPayload) override;

	//UIEvent StringToEnum(std::string aString);

	virtual std::string GetNodeName() const override { return "On Trigger Event"; }

	std::string GetNodeTypeCategory() const override { return "Events"; }

	bool IsStartNode() const override { return true; }

private:
	GraphNodeInstance* myNodeInstance = nullptr;
	std::string myEventName = "";
	int myReturnValue = -1;
	bool myClickedOn = false;
};