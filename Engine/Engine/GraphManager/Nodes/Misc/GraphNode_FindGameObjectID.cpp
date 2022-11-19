#include "pch.h"
#include "GraphNode_FindGameObjectID.h"
#include <iostream>
#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"
#include "Engine/GameObject/GameObject.h"


GraphNode_FindGameObjectID::GraphNode_FindGameObjectID()
{
	CreatePin("GameObject Name", PinDirection::PinDirection_IN, DataType::String);
	CreatePin("ID", PinDirection::PinDirection_OUT, DataType::Int);
}

int GraphNode_FindGameObjectID::OnExec(class GraphNodeInstance* aNodeInstance)
{
	auto text = GetPinData<std::string>(aNodeInstance, 0);
	auto aa = aNodeInstance->GetGameObject()->GetScene()->FindGameObject(text)->GetUUID();
	SetPinData<int>(aNodeInstance, 1, aa);
	return -1;
}
