#pragma once
#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h"
#include <Engine/GraphManager/Nodes/Event/NodeEventStructs.h>
#include <vector>

class Component;

class GraphNode_PlayAudioEvent : public GraphNodeBase
{
public:
	GraphNode_PlayAudioEvent();

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	virtual std::string GetNodeName() const override { return "Play Audio Event"; }

	std::string GetNodeTypeCategory() const override { return "Audio"; }
private:
};

