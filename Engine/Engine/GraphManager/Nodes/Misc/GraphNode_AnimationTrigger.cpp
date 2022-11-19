#include "pch.h"
#include "GraphNode_AnimationTrigger.h"
#include "Engine/Animation/AnimationController.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"

GraphNode_AnimationTrigger::GraphNode_AnimationTrigger()
{
	CreatePin("IN", PinDirection::PinDirection_IN, DataType::Exec);

	CreatePin("Trigger Name", PinDirection::PinDirection_IN, DataType::String);

	CreatePin("OUT", PinDirection::PinDirection_OUT, DataType::Exec);
}

int GraphNode_AnimationTrigger::OnExec(GraphNodeInstance* aNodeInstance)
{
	
	auto& animController = aNodeInstance->GetGameObject()->GetComponent<Engine::AnimatorComponent>()->GetController();
	auto trigger = GetPinData<std::string>(aNodeInstance, 1);

	animController.Trigger(trigger);

	return 2;
}
