#include "pch.h"
#include "GraphNode_ForLoop.h"

#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"

GraphNode_ForLoop::GraphNode_ForLoop()
{
	CreatePin("IN", PinDirection::PinDirection_IN, DataType::Exec);

	CreatePin("Start", PinDirection::PinDirection_IN, DataType::Int);
	CreatePin("End", PinDirection::PinDirection_IN, DataType::Int);

	CreatePin("Loop", PinDirection::PinDirection_OUT, DataType::Exec);
	CreatePin("Index", PinDirection::PinDirection_OUT, DataType::Int);

	CreatePin("Completed", PinDirection::PinDirection_OUT, DataType::Exec);
}

int GraphNode_ForLoop::OnExec(class GraphNodeInstance* aNodeInstance)
{
	const int start = GetPinData<int>(aNodeInstance, 1);
	const int end = GetPinData<int>(aNodeInstance, 2);

	auto& pins = aNodeInstance->GetPins();
	const bool onIterHasTarget = aNodeInstance->IsPinConnected(pins[1]);

	for (int i = start; i < end; ++i)
	{
		SetPinData(aNodeInstance, 4, i);

		aNodeInstance->ExitVia(3);
	}

	return 5;
}