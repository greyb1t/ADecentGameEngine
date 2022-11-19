#include "pch.h"
#include "GraphNode_OnTriggerEvent.h"
#include "Engine/GraphManager/GraphInstance.h"
#include "Engine/GameObject/GameObject.h"
#include <Engine/TriggerEventManager/TriggerEventManager.h>

GraphNode_OnTriggerEvent::GraphNode_OnTriggerEvent()
{
	CreatePin("Event", PinDirection::PinDirection_IN, DataType::String);
	CreatePin("OUT", PinDirection::PinDirection_OUT, DataType::Exec);
}
int GraphNode_OnTriggerEvent::OnExecWithPayload(class GraphNodeInstance* aNodeInstance, Payload& aPayload)
{
	myNodeInstance = aNodeInstance;
	auto stringEvent = GetPinData<std::string>(aNodeInstance, 0);
	//auto onThis = GetPinData<bool>(aNodeInstance, 1);

	/*if (onThis)
	{
		if (aPayload.GetEntry<int>(1) == myNodeInstance->GetGameObject()->GetUUID())
		{
			return 2;
		}
		else
		{
			return -1;
		}
	}*/

	if (aPayload.GetEntry<std::string>(0) == stringEvent && stringEvent != "")
	{
		return 1;
	}
	return -1;
}