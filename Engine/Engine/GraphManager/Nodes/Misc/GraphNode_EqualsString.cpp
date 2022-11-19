#include "pch.h"
#include "GraphNode_EqualsString.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"

GraphNode_EqualsString::GraphNode_EqualsString()
{
	CreatePin("IN", PinDirection::PinDirection_IN, DataType::Exec);

	CreatePin("String", PinDirection::PinDirection_IN, DataType::String);
	CreatePin("String", PinDirection::PinDirection_IN, DataType::String);
	
	CreatePin("True", PinDirection::PinDirection_OUT, DataType::Exec);
	CreatePin("False", PinDirection::PinDirection_OUT, DataType::Exec);
}

int GraphNode_EqualsString::OnExec(class GraphNodeInstance* aNodeInstance)
{
	const std::string value = GetPinData<std::string>(aNodeInstance, 1);
	const std::string value1 = GetPinData<std::string>(aNodeInstance, 2);

	if (value1 == value)
	{
		return 3;
	}
	else
	{
		return 4;
	}
}