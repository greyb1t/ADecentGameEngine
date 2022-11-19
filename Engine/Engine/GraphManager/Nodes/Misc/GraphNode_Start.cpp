#include "pch.h"
#include "GraphNode_Start.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"

GraphNode_Start::GraphNode_Start()
{
	CreatePin("", PinDirection::PinDirection_OUT);
}

int GraphNode_Start::OnExec(class GraphNodeInstance* aNodeInstance)
{
	return 0;
}
