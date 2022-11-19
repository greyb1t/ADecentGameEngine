#include "pch.h"
#include "GraphNode_ComponentGet.h"
#include "Engine/GameObject/Components/Component.h"
#include "Engine/Reflection/GraphNodeReflector.h"
#include "Engine/Reflection/SetPinDataReflector.h"
#include "Engine/GameObject/GameObjectManager.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/Scene/Scene.h"

GraphNode_ComponentGet::GraphNode_ComponentGet(const std::string& aEventName, Component* aComponent)
{
	CreatePin("Game Object ID", PinDirection::PinDirection_IN, DataType::Int);

	Engine::GraphNodeReflector graphNodeReflector(this, myDynamicPins, PinDirection::PinDirection_OUT);

	myComponentId = aComponent->GetComponentTypeID();
	myEventName = aEventName;

	aComponent->Reflect(graphNodeReflector);
}
int GraphNode_ComponentGet::OnExec(class GraphNodeInstance* aNodeInstance)
{
	const int id = GetPinData<int>(aNodeInstance, 0);

	GameObject* g = GScene->FindGameObject(id);

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
		Engine::SetPinDataReflector findValueReflector(this, aNodeInstance, myDynamicPins[i], i + 1);

		comp->Reflect(findValueReflector);
	}

	return -1;
}
