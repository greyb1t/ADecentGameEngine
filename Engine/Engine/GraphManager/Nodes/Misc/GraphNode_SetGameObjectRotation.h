#pragma once
#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h" 

class GraphNode_SetGameObjectRotation : public GraphNodeBase
{
public:
	GraphNode_SetGameObjectRotation();

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	virtual std::string GetNodeName() const override { return "Set GameObject Rotation"; }
	virtual std::string GetNodeTypeCategory() const override { return "Game Object"; }
};

