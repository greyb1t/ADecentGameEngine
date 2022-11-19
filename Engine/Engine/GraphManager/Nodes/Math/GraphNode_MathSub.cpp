#include "pch.h"
#include "GraphNode_MathSub.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"

GraphNode_MathSub::GraphNode_MathSub()
{
	CreatePin("A", PinDirection::PinDirection_IN, DataType::Float);
	CreatePin("B", PinDirection::PinDirection_IN, DataType::Float);
	CreatePin("Result", PinDirection::PinDirection_OUT, DataType::Float);
}

int GraphNode_MathSub::OnExec(class GraphNodeInstance* aNodeInstance)
{
	const float input1 = GetPinData<float>(aNodeInstance, 0);
	const float input2 = GetPinData<float>(aNodeInstance, 1);

	const float result = input1 - input2;

	SetPinData(aNodeInstance, 2, result);

	return -1;
}
