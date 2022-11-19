#include "pch.h"
#include "GraphNode_EqualsInt.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"

GraphNode_EqualsInt::GraphNode_EqualsInt()
{
	CreatePin("IN", PinDirection::PinDirection_IN, DataType::Exec);

	CreatePin("Int", PinDirection::PinDirection_IN, DataType::Int);
	CreatePin("Int", PinDirection::PinDirection_IN, DataType::Int);
	
	CreatePin("True", PinDirection::PinDirection_OUT, DataType::Exec);
	CreatePin("False", PinDirection::PinDirection_OUT, DataType::Exec);
}

int GraphNode_EqualsInt::OnExec(class GraphNodeInstance* aNodeInstance)
{
	const int value = GetPinData<int>(aNodeInstance, 1);
	const int value1 = GetPinData<int>(aNodeInstance, 2);

	if (value1 == value)
	{
		return 3;
	}
	else
	{
		return 4;
	}
}