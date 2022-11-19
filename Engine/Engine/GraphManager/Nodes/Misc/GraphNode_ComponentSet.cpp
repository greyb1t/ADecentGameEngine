#include "pch.h"
#include "GraphNode_ComponentSet.h"
#include "Engine/GameObject/Components/Component.h"
#include "Engine/Reflection/GraphNodeReflector.h"
#include "Engine/Reflection/SetComponentReflector.h"
#include "Engine/GameObject/GameObjectManager.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/Scene/Scene.h"
#include "ENgine/Scene/SceneManager.h"

GraphNode_ComponentSet::GraphNode_ComponentSet(const std::string& aEventName, Component* aComponent)
{
	CreatePin("IN", PinDirection::PinDirection_IN, DataType::Exec);

	CreatePin("Game Object ID", PinDirection::PinDirection_IN, DataType::Int);

	CreatePin("OUT", PinDirection::PinDirection_OUT, DataType::Exec);

	Engine::GraphNodeReflector graphNodeReflector(this, myDynamicPins, PinDirection::PinDirection_IN);

	myComponentId = aComponent->GetComponentTypeID();
	myEventName = aEventName;

	aComponent->Reflect(graphNodeReflector);
}
int GraphNode_ComponentSet::OnExec(class GraphNodeInstance* aNodeInstance)
{
	const int id = GetPinData<int>(aNodeInstance, 1);

	GameObject* g = aNodeInstance->GetGameObject()->GetScene()->FindGameObject(id);

	if (g == nullptr)
	{
		return -1;
	}

	auto comp = g->GetComponentByTypeId(myComponentId);

	if (comp == nullptr)
	{
		return -1;
	}
	for (int i = 0; i < myDynamicPins.size(); i++)
	{
		Engine::SetComponentReflector setComponentReflector(this, aNodeInstance, myDynamicPins[i], i + 3);

		comp->Reflect(setComponentReflector);
	}

	return 2;
}
