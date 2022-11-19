#include "pch.h"
#include "GraphNode_Branch.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"

GraphNode_Branch::GraphNode_Branch()
{
	CreatePin("IN", PinDirection::PinDirection_IN, DataType::Exec);
	CreatePin("True", PinDirection::PinDirection_OUT, DataType::Exec);
	CreatePin("False", PinDirection::PinDirection_OUT, DataType::Exec);

	CreatePin("Condition", PinDirection::PinDirection_IN, DataType::Bool);
}

int GraphNode_Branch::OnExec(class GraphNodeInstance* aNodeInstance)
{
	const bool condition = GetPinData<bool>(aNodeInstance, 3);

	if (condition)
	{
		return 1;
	}
	else
	{
		return 2;
	}
}