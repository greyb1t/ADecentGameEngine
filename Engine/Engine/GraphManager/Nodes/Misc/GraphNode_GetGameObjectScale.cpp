#include "pch.h"
#include "Engine/Engine.h"
#include "Engine/Scene/Scene.h"
#include "GraphNode_GetGameObjectScale.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"
#include "Engine/GameObject/GameObject.h"

GraphNode_GetGameObjectScale::GraphNode_GetGameObjectScale()
{
	CreatePin("UUID", PinDirection::PinDirection_IN, DataType::Int);

	CreatePin("ScaleX", PinDirection::PinDirection_OUT, DataType::Float);
	CreatePin("ScaleY", PinDirection::PinDirection_OUT, DataType::Float);
	CreatePin("ScaleZ", PinDirection::PinDirection_OUT, DataType::Float);
}

int GraphNode_GetGameObjectScale::OnExec(class GraphNodeInstance* aNodeInstance)
{
	const int uuid = GetPinData<int>(aNodeInstance, 0);
	auto gameObject = GScene->FindGameObject(uuid);

	if (gameObject == nullptr) {
		LOG_WARNING(LogType::Components) << "Gaem Object NUll. GraphNode_GetGameObjectScale.cpp";
		return -1;
	}

	const Vec3f scale = gameObject->GetTransform().GetScale();

	SetPinData(aNodeInstance, 1, scale.x);
	SetPinData(aNodeInstance, 2, scale.y);
	SetPinData(aNodeInstance, 3, scale.z);

	return -1;
}