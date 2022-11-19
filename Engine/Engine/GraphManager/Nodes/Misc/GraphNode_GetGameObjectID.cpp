#include "pch.h"
#include "GraphNode_GetGameObjectID.h"
#include <iostream>
#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"
#include "Engine/GameObject/GameObject.h"


GraphNode_GetGameObjectID::GraphNode_GetGameObjectID()
{
	CreatePin("ID", PinDirection::PinDirection_OUT, DataType::Int);
}

int GraphNode_GetGameObjectID::OnExec(class GraphNodeInstance* aNodeInstance)
{
	SetPinData<int>(aNodeInstance, 0, aNodeInstance->GetGameObject()->GetUUID());
	return -1;
}
