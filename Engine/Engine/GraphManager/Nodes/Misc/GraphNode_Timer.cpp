#include "pch.h"
#include "GraphNode_Timer.h"
// #include "GameObjectManager.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"
#include <Engine/TimeSystem/CallbackTimer.h>
// #include "TimerManager.h"

GraphNode_Timer::GraphNode_Timer()
{
	CreatePin("EXEC", PinDirection::PinDirection_IN, DataType::Exec);
	CreatePin("OUT", PinDirection::PinDirection_OUT, DataType::Exec);

	CreatePin("Duration", PinDirection::PinDirection_IN, DataType::Float);
	CreatePin("Loop", PinDirection::PinDirection_IN, DataType::Bool);

	CreatePin("On Timer", PinDirection::PinDirection_OUT, DataType::Exec);

	//CreatePin("Timer Handle", PinDirection::PinDirection_OUT, DataType::Int);
}

void GraphNode_Timer::OnTimerElapsed(GraphNodeInstance* aNodeInstance)
{
	aNodeInstance->ExitVia(4);
}

int GraphNode_Timer::OnExec(class GraphNodeInstance* aNodeInstance)
{
	const float duration = GetPinData<float>(aNodeInstance, 2);
	const bool loop = GetPinData<bool>(aNodeInstance, 3);

	DBG_NEW Engine::CallbackTimer([this, aNodeInstance]()
	{
		OnTimerElapsed(aNodeInstance);
	}, duration);
	//SetPinData(aNodeInstance, 5, timer.);

	return 1;
}

