#pragma once

#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h"
#include "NodeEventStructs.h"
#include "Engine/UIEventManager/UIEventManager.h"
#include <string>

class GraphNode_OnButtonHoverExit : public GraphNodeBase
{
public:
	GraphNode_OnButtonHoverExit();

	int OnExecWithPayload(GraphNodeInstance* aNodeInstance, Payload& aPayload) override;

	//UIEvent StringToEnum(std::string aString);

	virtual std::string GetNodeName() const override { return "On Button Hover Exit"; }

	std::string GetNodeTypeCategory() const override { return "Events"; }

	bool IsStartNode() const override { return true; }

private:
	GraphNodeInstance* myNodeInstance = nullptr;
	std::string myEventName = "";
	int myReturnValue = -1;
	bool myHoverExitOn = false;
};