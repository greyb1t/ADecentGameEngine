#pragma once
#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h"

class GraphNode_Set : public GraphNodeBase
{

public:
	GraphNode_Set();

	virtual int OnExec(GraphNodeInstance* aNodeInstance) override;
	virtual std::string GetInstanceName(const GraphNodeInstance* aNodeInstance) const override;
};

class GraphNode_Get : public GraphNodeBase
{
public:

	GraphNode_Get();

	virtual int OnExec(GraphNodeInstance* aNodeInstance) override;
	virtual std::string GetInstanceName(const GraphNodeInstance* aNodeInstance) const override;
};

