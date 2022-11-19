#pragma once
#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h" 

class GameObject;

class GraphNode_FadeAllChildren : public GraphNodeBase
{
public:
	GraphNode_FadeAllChildren();

	int OnExec(class GraphNodeInstance* aNodeInstance) override;
	void FadeChildren(GameObject* aGo, float aSpeed, GraphNodeInstance* aNodeInstance);

	virtual std::string GetNodeName() const override { return "Fade All Children"; }
	virtual std::string GetNodeTypeCategory() const override { return "Misc"; }
};

