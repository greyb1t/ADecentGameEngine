#include "pch.h"
#include "Engine/Engine.h"
#include "Engine/Scene/Scene.h"
#include "GraphNode_SetGameObjectPosition.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"
#include "Engine/GameObject/GameObject.h"

GraphNode_SetGameObjectPosition::GraphNode_SetGameObjectPosition()
{
	CreatePin("IN", PinDirection::PinDirection_IN, DataType::Exec);

	CreatePin("UUID", PinDirection::PinDirection_IN, DataType::Int);

	CreatePin("PosX", PinDirection::PinDirection_IN, DataType::Float);
	CreatePin("PosY", PinDirection::PinDirection_IN, DataType::Float);
	CreatePin("PosZ", PinDirection::PinDirection_IN, DataType::Float);

	CreatePin("OUT", PinDirection::PinDirection_OUT, DataType::Exec);
}

int GraphNode_SetGameObjectPosition::OnExec(class GraphNodeInstance* aNodeInstance)
{
	const int uuid = GetPinData<int>(aNodeInstance, 1);
	auto gameObject = aNodeInstance->GetGameObject()->GetScene()->FindGameObject(uuid);

	if (gameObject == nullptr) {
		LOG_WARNING(LogType::Components) << "Gaem Object NUll. GraphNode_SetGameObjectPosition.cpp";
		return -1;
	}

	Vec3f pos;
	
	pos.x = GetPinData<float>(aNodeInstance, 2);
	pos.y = GetPinData<float>(aNodeInstance, 3);
	pos.z = GetPinData<float>(aNodeInstance, 4);

	gameObject->GetTransform().SetPosition(pos);

	return 5;
}