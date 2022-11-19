#pragma once
#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h" 

class GraphNode_SetMaterialValue : public GraphNodeBase
{
public:
	GraphNode_SetMaterialValue();

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	virtual std::string GetNodeName() const override { return "Set Material Value"; }
	virtual std::string GetNodeTypeCategory() const override { return "Material"; }
};

