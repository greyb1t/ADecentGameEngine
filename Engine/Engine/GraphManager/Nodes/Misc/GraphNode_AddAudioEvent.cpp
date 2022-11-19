#include "pch.h"
#include "GraphNode_AddAudioEvent.h"
#include "Engine/GameObject/Components/Component.h"
#include "Engine/Reflection/GraphNodeReflector.h"
#include "Engine/Reflection/SetComponentReflector.h"
#include "Engine/GameObject/GameObjectManager.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"
#include "Engine/GameObject/Components/AudioComponent.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/Scene/SceneManager.h"
#include "Engine/Scene/Scene.h"

GraphNode_AddAudioEvent::GraphNode_AddAudioEvent()
{
	CreatePin("IN", PinDirection::PinDirection_IN, DataType::Exec);

	CreatePin("Game Object ID", PinDirection::PinDirection_IN, DataType::Int);
	CreatePin("EventName", PinDirection::PinDirection_IN, DataType::String);
	CreatePin("EventPath", PinDirection::PinDirection_IN, DataType::String);

	CreatePin("OUT", PinDirection::PinDirection_OUT, DataType::Exec);
}
int GraphNode_AddAudioEvent::OnExec(class GraphNodeInstance* aNodeInstance)
{
	const int id = GetPinData<int>(aNodeInstance, 1);

	GameObject* g = aNodeInstance->GetGameObject()->GetScene()->FindGameObject(id);

	if (g == nullptr)
	{
		return -1;
	}

	auto audioComp = g->GetComponent<Engine::AudioComponent>();

	const std::string eventName = GetPinData<std::string>(aNodeInstance, 2);
	const std::string eventPath = GetPinData<std::string>(aNodeInstance, 3);

	audioComp->AddEvent(eventName, eventPath);

	return 4;
}
