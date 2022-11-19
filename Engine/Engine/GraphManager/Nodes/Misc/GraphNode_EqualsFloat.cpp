#include "pch.h"
#include "GraphNode_EqualsFloat.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"

GraphNode_EqualsFloat::GraphNode_EqualsFloat()
{
	CreatePin("IN", PinDirection::PinDirection_IN, DataType::Exec);

	CreatePin("Float", PinDirection::PinDirection_IN, DataType::Float);
	CreatePin("Float2", PinDirection::PinDirection_IN, DataType::Float);
	
	CreatePin("True", PinDirection::PinDirection_OUT, DataType::Exec);
	CreatePin("False", PinDirection::PinDirection_OUT, DataType::Exec);
}

int GraphNode_EqualsFloat::OnExec(class GraphNodeInstance* aNodeInstance)
{
	const float value = GetPinData<float>(aNodeInstance, 1);
	const float value1 = GetPinData<float>(aNodeInstance, 2);

	if (value1 == value)
	{
		return 3;
	}
	else
	{
		return 4;
	}
}