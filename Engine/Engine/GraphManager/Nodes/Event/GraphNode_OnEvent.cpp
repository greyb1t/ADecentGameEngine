#include "pch.h"
#include "GraphNode_OnEvent.h"

#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"

GraphNode_OnEvent::GraphNode_OnEvent(const std::string& aEventName, const std::vector<DynamicPin>& aDynamicPins)
	: myEventName(aEventName),
	myDynamicPins(aDynamicPins)
{
	CreatePin("OUT", PinDirection::PinDirection_OUT, DataType::Exec);

	for (const auto& pin : myDynamicPins)
	{
		CreatePin(pin.myName, PinDirection::PinDirection_OUT, pin.myDataType);
	}
}

int GraphNode_OnEvent::OnExecWithPayload(GraphNodeInstance* aNodeInstance, Payload& aPayload)
{
	for (size_t i = 0; i < myDynamicPins.size(); ++i)
	{
		const int pinIndex = i + 1;

		switch (myDynamicPins[i].myDataType) {
		case DataType::Bool:
		{
			bool v = aPayload.GetEntry<bool>(i);
			SetPinData(aNodeInstance, pinIndex, v);
		} break;
		case DataType::Int:
		{
			int v = aPayload.GetEntry<int>(i);
			SetPinData(aNodeInstance, pinIndex, v);
		} break;
		case DataType::Float:
		{
			float v = aPayload.GetEntry<float>(i);
			SetPinData(aNodeInstance, pinIndex, v);
		} break;
		case DataType::String:
		{
			std::string v = aPayload.GetEntry<std::string>(i);
			SetPinData(aNodeInstance, pinIndex, v);
		} break;
		default:
			assert(false && "unhandled");
			break;
		}
	}

	return 0;
}
