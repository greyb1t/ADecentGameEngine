#include "pch.h"
#include "pch.h"
#include "GraphNode_Clamp.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"

GraphNode_MathClamp::GraphNode_MathClamp()
{
	CreatePin("Input", PinDirection::PinDirection_IN, DataType::Float);
	CreatePin("Min", PinDirection::PinDirection_IN, DataType::Float);
	CreatePin("Max", PinDirection::PinDirection_IN, DataType::Float);
	CreatePin("Result", PinDirection::PinDirection_OUT, DataType::Float);
}

int GraphNode_MathClamp::OnExec(class GraphNodeInstance* aNodeInstance)
{
	float input = GetPinData<float>(aNodeInstance, 0);
	const float min = GetPinData<float>(aNodeInstance, 1);
	const float max = GetPinData<float>(aNodeInstance, 2);

	if (input <= min) {
		input = min;
	}
	if (input >= max) {
		input = max;
	}

	SetPinData(aNodeInstance, 3, input);

	return -1;
}

