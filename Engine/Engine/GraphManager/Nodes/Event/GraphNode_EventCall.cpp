#include "pch.h"
#include "GraphNode_EventCall.h"

#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"

GraphNode_EventCall::GraphNode_EventCall(
	const std::string& aEventName, const std::vector<DynamicPin>& aDynamicPins)
	: myEventName(aEventName)
	, myDynamicPins(aDynamicPins)
{
	CreatePin("ID", PinDirection::PinDirection_IN, DataType::Int);

	CreatePin("IN", PinDirection::PinDirection_IN, DataType::Exec);

	CreatePin("OUT", PinDirection::PinDirection_OUT, DataType::Exec);

	for (const auto& pin : myDynamicPins)
	{
		CreatePin(pin.myName, PinDirection::PinDirection_IN, pin.myDataType);
	}
}

int GraphNode_EventCall::OnExec(class GraphNodeInstance* aNodeInstance)
{
	const int id = GetPinData<int>(aNodeInstance, 0);

	// TODO: When this is to be implemented, uncomment this line
	// and the line below that calls the GameObject's Graph
	// I dont think we should use this method to call on GameObject, since
	// our graphs lay on Component and stuff
	// Maybe simply send the event in the PostMaster instead?
	// Much better because we can ourself in code also respond on the events
	// not just be limited to responding to the event in graphs.
	// GameObject* g = GameObjectManager::GetGameObject(id);

	Payload payload;

	for (size_t i = 0; i < myDynamicPins.size(); ++i)
	{
		DataType dt;
		DataPtr dp;
		size_t sz;
		GetPinData(aNodeInstance, i + 3, dt, dp, sz);

		switch (dt)
		{
			case DataType::Bool:
				payload.AddData<DataType::Bool>(dp.Get<bool>());
				break;
			case DataType::Int:
				payload.AddData<DataType::Int>(dp.Get<int>());
				break;
			case DataType::Float:
				payload.AddData<DataType::Float>(dp.Get<float>());
				break;
			case DataType::String:
				payload.AddData<DataType::String>(dp.Get<std::string>());
				break;
			default:
				assert(false && "unhandled");
				break;
		}
	}

	//g->GetGraphInstance()->ExecuteTreeWithPayload("On " + myEventName, payload);

	return 2;
}