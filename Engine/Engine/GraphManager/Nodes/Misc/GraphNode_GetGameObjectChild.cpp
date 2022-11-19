#include "pch.h"
#include "Engine/Engine.h"
#include "Engine/Scene/Scene.h"
#include "GraphNode_GetGameObjectChild.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"
#include "Engine/GameObject/GameObject.h"

GraphNode_GetGameObjectChild::GraphNode_GetGameObjectChild()
{
	CreatePin("Child Index", PinDirection::PinDirection_IN, DataType::Int);
	CreatePin("Child UUID", PinDirection::PinDirection_OUT, DataType::Int);
}

int GraphNode_GetGameObjectChild::OnExec(class GraphNodeInstance* aNodeInstance)
{
	auto index = GetPinData<int>(aNodeInstance, 0);
	SetPinData<int>(aNodeInstance, 1, aNodeInstance->GetGameObject()->GetTransform().GetChildren()[index]->GetGameObject()->GetUUID());

	return -1;
}