#include "pch.h"
#include "GraphNode_MathLength.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"

GraphNode_MathLength::GraphNode_MathLength()
{
	CreatePin("X1", PinDirection::PinDirection_IN, DataType::Float);
	CreatePin("Y1", PinDirection::PinDirection_IN, DataType::Float);
	CreatePin("X2", PinDirection::PinDirection_IN, DataType::Float);
	CreatePin("Y2", PinDirection::PinDirection_IN, DataType::Float);

	CreatePin("Result", PinDirection::PinDirection_OUT, DataType::Float);
}

int GraphNode_MathLength::OnExec(class GraphNodeInstance* aNodeInstance)
{
	const float x1 = GetPinData<float>(aNodeInstance, 0);
	const float y1 = GetPinData<float>(aNodeInstance, 1);

	const float x2 = GetPinData<float>(aNodeInstance, 2);
	const float y2 = GetPinData<float>(aNodeInstance, 3);

	CU::Vector2f v1(x1, y1);
	CU::Vector2f v2(x2, y2);

	const float length = (v2 - v1).Length();

	// Set the result we calculated to the data property on the Result pin.
	SetPinData(aNodeInstance, 4, length);

	return -1; // If the node doesn't have a flow, return -1 see the print node for reference
}

