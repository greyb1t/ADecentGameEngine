#include "pch.h"
#include "GraphNode_SetParameter.h"
#include "Engine/Animation/AnimationController.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include <Engine/AudioManager.h>

GraphNode_SetParameter::GraphNode_SetParameter()
{
	CreatePin("IN", PinDirection::PinDirection_IN, DataType::Exec);

	CreatePin("Parameter", PinDirection::PinDirection_IN, DataType::String);
	CreatePin("Value", PinDirection::PinDirection_IN, DataType::Float);

	CreatePin("OUT", PinDirection::PinDirection_OUT, DataType::Exec);
}

int GraphNode_SetParameter::OnExec(GraphNodeInstance* aNodeInstance)
{
	auto parameter = GetPinData<std::string>(aNodeInstance, 1);
	auto value = GetPinData<bool>(aNodeInstance, 2);

	AudioManager::GetInstance()->ChangeParameter(parameter, value);
	
	return 3;
}
