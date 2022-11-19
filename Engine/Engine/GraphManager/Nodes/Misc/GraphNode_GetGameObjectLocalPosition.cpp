#include "pch.h"
#include "Engine/Engine.h"
#include "Engine/Scene/Scene.h"
#include "GraphNode_GetGameObjectLocalPosition.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"
#include "Engine/GameObject/GameObject.h"

GraphNode_GetGameObjectLocalPosition::GraphNode_GetGameObjectLocalPosition()
{
	CreatePin("UUID", PinDirection::PinDirection_IN, DataType::Int);

	CreatePin("PosX", PinDirection::PinDirection_OUT, DataType::Float);
	CreatePin("PosY", PinDirection::PinDirection_OUT, DataType::Float);
	CreatePin("PosZ", PinDirection::PinDirection_OUT, DataType::Float);
}

int GraphNode_GetGameObjectLocalPosition::OnExec(class GraphNodeInstance* aNodeInstance)
{
	const int uuid = GetPinData<int>(aNodeInstance, 0);
	auto gameObject = aNodeInstance->GetGameObject()->GetScene()->FindGameObject(uuid);

	if (gameObject == nullptr) {
		LOG_WARNING(LogType::Components) << "Gaem Object NUll. GraphNode_GetGameObjectLocalPosition.cpp";
		return -1;
	}

	const Vec3f pos = gameObject->GetTransform().GetPosition();

	SetPinData(aNodeInstance, 1, pos.x);
	SetPinData(aNodeInstance, 2, pos.y);
	SetPinData(aNodeInstance, 3, pos.z);

	return -1;
}