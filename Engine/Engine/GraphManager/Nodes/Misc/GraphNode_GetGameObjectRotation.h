#pragma once
#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h" 

class GraphNode_GetGameObjectRotation : public GraphNodeBase
{
public:
	GraphNode_GetGameObjectRotation();

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	virtual std::string GetNodeName() const override { return "Get GameObject Rotation"; }
	virtual std::string GetNodeTypeCategory() const override { return "Game Object"; }
};

