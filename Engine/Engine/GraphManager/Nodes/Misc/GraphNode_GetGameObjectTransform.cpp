#include "pch.h"
#include "Engine/Engine.h"
#include "Engine/Scene/Scene.h"
#include "GraphNode_GetGameObjectTransform.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"
#include "Engine/GameObject/GameObject.h"

GraphNode_GetGameObjectTransform::GraphNode_GetGameObjectTransform()
{
	CreatePin("UUID", PinDirection::PinDirection_IN, DataType::Int);

	CreatePin("PosX", PinDirection::PinDirection_OUT, DataType::Float);
	CreatePin("PosY", PinDirection::PinDirection_OUT, DataType::Float);
	CreatePin("PosZ", PinDirection::PinDirection_OUT, DataType::Float);

	CreatePin("RotX", PinDirection::PinDirection_OUT, DataType::Float);
	CreatePin("RotY", PinDirection::PinDirection_OUT, DataType::Float);
	CreatePin("RotZ", PinDirection::PinDirection_OUT, DataType::Float);

	CreatePin("ScaleX", PinDirection::PinDirection_OUT, DataType::Float);
	CreatePin("ScaleY", PinDirection::PinDirection_OUT, DataType::Float);
	CreatePin("ScaleZ", PinDirection::PinDirection_OUT, DataType::Float);
}

int GraphNode_GetGameObjectTransform::OnExec(class GraphNodeInstance* aNodeInstance)
{
	const int uuid = GetPinData<int>(aNodeInstance, 0);
	auto gameObject = GScene->FindGameObject(uuid);

	if (gameObject == nullptr) {
		LOG_WARNING(LogType::Components) << "Gaem Object NUll. GraphNode_GetGameObjectTransform.cpp";
		return -1;
	}
	const Vec3f pos = gameObject->GetTransform().GetPosition();

	const Vec3f rot = gameObject->GetTransform().GetRotation().EulerAngles();

	const Vec3f scale = gameObject->GetTransform().GetScale();

	SetPinData(aNodeInstance, 1, pos.x);
	SetPinData(aNodeInstance, 2, pos.y);
	SetPinData(aNodeInstance, 3, pos.z);

	SetPinData(aNodeInstance, 4, rot.x);
	SetPinData(aNodeInstance, 5, rot.y);
	SetPinData(aNodeInstance, 6, rot.z);

	SetPinData(aNodeInstance, 7, scale.x);
	SetPinData(aNodeInstance, 8, scale.y);
	SetPinData(aNodeInstance, 9, scale.z);

	return -1;
}