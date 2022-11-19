#include "pch.h"
#include "GraphNode_StopTimer.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"

GraphNode_StopTimer::GraphNode_StopTimer()
{
	// CreatePin("IN", PinDirection::PinDirection_IN, DataType::Exec);
	// CreatePin("OUT", PinDirection::PinDirection_OUT, DataType::Exec);
	// 
	// CreatePin("Timer Handle", PinDirection::PinDirection_IN, DataType::Int);
}

GraphNode_StopTimer::~GraphNode_StopTimer()
{
}

int GraphNode_StopTimer::OnExec(class GraphNodeInstance* aNodeInstance)
{
	//const int timerId = GetPinData<int>(aNodeInstance, 2);

	// TimerManager::ClearTimer(timerId);

	return 1;
}
