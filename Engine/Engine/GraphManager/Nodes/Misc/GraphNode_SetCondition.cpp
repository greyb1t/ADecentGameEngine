#include "pch.h"
#include "GraphNode_SetCondition.h"
#include "Engine/Animation/AnimationController.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"

GraphNode_SetCondition::GraphNode_SetCondition()
{
	CreatePin("IN", PinDirection::PinDirection_IN, DataType::Exec);

	CreatePin("Condition", PinDirection::PinDirection_IN, DataType::String);
	CreatePin("Value", PinDirection::PinDirection_IN, DataType::Bool);

	CreatePin("OUT", PinDirection::PinDirection_OUT, DataType::Exec);
}

int GraphNode_SetCondition::OnExec(GraphNodeInstance* aNodeInstance)
{
	auto& animController = aNodeInstance->GetGameObject()->GetComponent<Engine::AnimatorComponent>()->GetController();
	auto condition = GetPinData<std::string>(aNodeInstance, 1);
	auto value = GetPinData<bool>(aNodeInstance, 2);

	animController.SetBool(condition, value);
	
	return 3;
}
