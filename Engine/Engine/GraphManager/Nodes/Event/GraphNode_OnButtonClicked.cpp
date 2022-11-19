#include "pch.h"
#include "GraphNode_OnButtonClicked.h"
#include "Engine/GraphManager/GraphInstance.h"
#include "Engine/GameObject/GameObject.h"

GraphNode_OnButtonClicked::GraphNode_OnButtonClicked()
{
	CreatePin("Event", PinDirection::PinDirection_IN, DataType::String);
	CreatePin("On This Object", PinDirection::PinDirection_IN, DataType::Bool);
	CreatePin("OUT", PinDirection::PinDirection_OUT, DataType::Exec);
	//UIEventManager::GetInstance().Subscribe(this, UIEvent::ButtonClicked);
}
int GraphNode_OnButtonClicked::OnExecWithPayload(class GraphNodeInstance* aNodeInstance, Payload& aPayload)
{
	myNodeInstance = aNodeInstance;
	auto stringEvent = GetPinData<std::string>(aNodeInstance, 0);
	auto onThis = GetPinData<bool>(aNodeInstance, 1);

	if (onThis)
	{
		if (aPayload.GetEntry<int>(1) == myNodeInstance->GetGameObject()->GetUUID())
		{
			return 2;
		}
		else
		{
			return -1;
		}
	}

	if (aPayload.GetEntry<std::string>(0) == stringEvent && stringEvent != "")
	{
		return 2;
	}
	return -1;
}