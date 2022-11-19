#include "pch.h"
#include "GraphNode_MathSin.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"

GraphNode_MathSin::GraphNode_MathSin()
{
	CreatePin("X", PinDirection::PinDirection_IN, DataType::Float);
	CreatePin("Result", PinDirection::PinDirection_OUT, DataType::Float);
}

int GraphNode_MathSin::OnExec(class GraphNodeInstance* aNodeInstance)
{
	// Get data on pin index 0, this index is relative to what you push in the constructor
	const float input = GetPinData<float>(aNodeInstance, 0);

	// Perform the Add operation we're supposed to do.
	const float result = sin(input);

	// Set the result we calculated to the data property on the Result pin.
	SetPinData(aNodeInstance, 1, result);

	return -1; // If the node doesn't have a flow, return -1 see the print node for reference
}

