#include "pch.h"
#include "GraphNode_OnInput.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"

GraphNode_OnInput::GraphNode_OnInput(
	const std::string& aEventName, const std::vector<DynamicPin>& aDynamicPins)
	: myEventName(aEventName)
	, myDynamicPins(aDynamicPins)
{
	CreatePin("ID", PinDirection::PinDirection_IN, DataType::Int);
}

int GraphNode_OnInput::OnExec(class GraphNodeInstance* aNodeInstance)
{
	const int id = GetPinData<int>(aNodeInstance, 0);

	// REFLECT IN NODE EDITOR SOMEHOW
	if (ImGui::BeginCombo("", std::to_string(myCurrentKey).c_str()))
	{
		for (const auto [hex, key] : myKeys)
		{
			bool selected = hex == myCurrentKey;
			if (ImGui::Selectable(key.c_str(), selected)) 
			{
				myCurrentKey = hex;
			}
		}
	}

	return 2;
}
