#pragma once

#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h"
#include "NodeEventStructs.h"
#include "Engine/UIEventManager/UIEventManager.h"
//#include "Engine/UIEventManager/UIEventListener.h"
#include <string>

class GraphNode_OnButtonHoverEnter : public GraphNodeBase
{
public:
	GraphNode_OnButtonHoverEnter();

	int OnExecWithPayload(GraphNodeInstance* aNodeInstance, Payload& aPayload) override;

	//UIEvent StringToEnum(std::string aString);

	virtual std::string GetNodeName() const override { return "On Button Hover Enter"; }

	std::string GetNodeTypeCategory() const override { return "Events"; }

	bool IsStartNode() const override { return true; }

private:
	GraphNodeInstance* myNodeInstance = nullptr;
	std::string myEventName = "";
	int myReturnValue = -1;
	bool myHoverEnterOn = false;
};