#include "pch.h"
#include "GraphNode_OnButtonHoverEnter.h"
#include "Engine/GraphManager/GraphInstance.h"
#include "Engine/GameObject/GameObject.h"

GraphNode_OnButtonHoverEnter::GraphNode_OnButtonHoverEnter()
{
	CreatePin("Event", PinDirection::PinDirection_IN, DataType::String);
	CreatePin("On This Object", PinDirection::PinDirection_IN, DataType::Bool);
	CreatePin("OUT", PinDirection::PinDirection_OUT, DataType::Exec);
	//UIEventManager::GetInstance().Subscribe(this, UIEvent::ButtonHoverEnter);
}
int GraphNode_OnButtonHoverEnter::OnExecWithPayload(class GraphNodeInstance* aNodeInstance, Payload& aPayload)
{
	myNodeInstance = aNodeInstance;
	auto stringEvent = GetPinData<std::string>(aNodeInstance, 0);
	auto onThis = GetPinData<bool>(aNodeInstance, 1);

	if (onThis)
	{
		int a = myNodeInstance->GetGameObject()->GetUUID();
		int b = aPayload.GetEntry<int>(1);
		if (aPayload.GetEntry<int>(1) == myNodeInstance->GetGameObject()->GetUUID())
		{
			return 2;
		}
		else 
		{
			return -1;
		}
	}

	if (aPayload.GetEntry<std::string>(0) == stringEvent)
	{
		return 2;
	}
	return -1;
}