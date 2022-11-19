#pragma once
#pragma once
#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h" 

class GraphNode_AnimationTrigger : public GraphNodeBase
{
public:
	GraphNode_AnimationTrigger();

	int OnExec(class GraphNodeInstance* aNodeInstance) override;

	virtual std::string GetNodeName() const override { return "Animation Trigger"; }
	virtual std::string GetNodeTypeCategory() const override { return "Animation"; }
};

