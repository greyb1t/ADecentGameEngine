#include "pch.h"
#include "Engine/Engine.h"
#include "Engine/Scene/Scene.h"
#include "GraphNode_GetGameObjectParent.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"
#include "Engine/GameObject/GameObject.h"

GraphNode_GetGameObjectParent::GraphNode_GetGameObjectParent()
{
	CreatePin("Parent UUID", PinDirection::PinDirection_OUT, DataType::Int);
}

int GraphNode_GetGameObjectParent::OnExec(class GraphNodeInstance* aNodeInstance)
{
	SetPinData<int>(aNodeInstance, 0, aNodeInstance->GetGameObject()->GetTransform().GetParent()->GetGameObject()->GetUUID());

	return -1;
}