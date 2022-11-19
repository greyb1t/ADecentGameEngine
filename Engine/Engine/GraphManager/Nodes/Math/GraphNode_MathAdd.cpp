#include "pch.h"
#include "GraphNode_MathAdd.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"

GraphNode_MathAdd::GraphNode_MathAdd()
{
	CreatePin("A", PinDirection::PinDirection_IN, DataType::Float);
	CreatePin("B", PinDirection::PinDirection_IN, DataType::Float);
	CreatePin("Result", PinDirection::PinDirection_OUT, DataType::Float);
}

int GraphNode_MathAdd::OnExec(class GraphNodeInstance* aNodeInstance)
{
	// Get data on pin index 0, this index is relative to what you push in the constructor
	const float input1 = GetPinData<float>(aNodeInstance, 0);

	// Get data on pin index 1, this index is relative to what you push in the constructor
	const float input2 = GetPinData<float>(aNodeInstance, 1);

	// Perform the Add operation we're supposed to do.
	const float result = input1 + input2;

	// Set the result we calculated to the data property on the Result pin.
	SetPinData(aNodeInstance, 2, result);	

	return -1; // If the node doesn't have a flow, return -1 see the print node for reference
}

