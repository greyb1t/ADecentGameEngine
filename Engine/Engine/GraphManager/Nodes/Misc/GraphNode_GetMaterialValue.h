#pragma once
#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h" 

class GraphNode_GetMaterialValue : public GraphNodeBase
{
public:
	GraphNode_GetMaterialValue();

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	virtual std::string GetNodeName() const override { return "Get Material Value"; }
	virtual std::string GetNodeTypeCategory() const override { return "Material"; }
};

