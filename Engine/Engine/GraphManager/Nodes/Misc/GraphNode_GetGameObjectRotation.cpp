#include "pch.h"
#include "Engine/Engine.h"
#include "Engine/Scene/Scene.h"
#include "GraphNode_GetGameObjectRotation.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"
#include "Engine/GameObject/GameObject.h"

GraphNode_GetGameObjectRotation::GraphNode_GetGameObjectRotation()
{
	CreatePin("UUID", PinDirection::PinDirection_IN, DataType::Int);

	CreatePin("RotX", PinDirection::PinDirection_OUT, DataType::Float);
	CreatePin("RotY", PinDirection::PinDirection_OUT, DataType::Float);
	CreatePin("RotZ", PinDirection::PinDirection_OUT, DataType::Float);
}

int GraphNode_GetGameObjectRotation::OnExec(class GraphNodeInstance* aNodeInstance)
{
	const int uuid = GetPinData<int>(aNodeInstance, 0);
	auto gameObject = GScene->FindGameObject(uuid);

	if (gameObject == nullptr) {
		LOG_WARNING(LogType::Components) << "Gaem Object NUll. GraphNode_GetGameObjectRotation.cpp";
		return -1;
	}

	const Vec3f rot = gameObject->GetTransform().GetRotation().EulerAngles();

	SetPinData(aNodeInstance, 1, rot.x);
	SetPinData(aNodeInstance, 2, rot.y);
	SetPinData(aNodeInstance, 3, rot.z);

	return -1;
}