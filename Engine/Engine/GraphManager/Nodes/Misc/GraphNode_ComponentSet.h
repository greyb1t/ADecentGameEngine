#pragma once
#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h"
#include <Engine/GraphManager/Nodes/Event/NodeEventStructs.h>
#include <vector>

class Component;

class GraphNode_ComponentSet : public GraphNodeBase
{
public:
	GraphNode_ComponentSet(const std::string& aEventName, Component* aComponent);

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	virtual std::string GetNodeName() const override { return "Set Component " + myEventName; }

	std::string GetNodeTypeCategory() const override { return "Component"; }
private:
	std::vector<DynamicPin> myDynamicPins;
	std::string myEventName;
	int myComponentId = -1;
};

