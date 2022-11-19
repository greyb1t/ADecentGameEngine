#include "pch.h"
#include "GraphNode_MathMin.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"

GraphNode_MathMin::GraphNode_MathMin()
{
	CreatePin("X", PinDirection::PinDirection_IN, DataType::Float);
	CreatePin("Y", PinDirection::PinDirection_IN, DataType::Float);
	CreatePin("Result", PinDirection::PinDirection_OUT, DataType::Float);
}

int GraphNode_MathMin::OnExec(class GraphNodeInstance* aNodeInstance)
{
	const float input1 = GetPinData<float>(aNodeInstance, 0);
	const float input2 = GetPinData<float>(aNodeInstance, 1);

	// Perform the Add operation we're supposed to do.
	const float result = std::min(input1, input2);

	// Set the result we calculated to the data property on the Result pin.
	SetPinData(aNodeInstance, 2, result);

	return -1; // If the node doesn't have a flow, return -1 see the print node for reference
}

