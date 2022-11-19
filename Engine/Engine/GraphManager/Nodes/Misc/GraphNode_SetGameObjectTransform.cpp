#include "pch.h"
#include "Engine/Engine.h"
#include "Engine/Scene/Scene.h"
#include "GraphNode_SetGameObjectTransform.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"
#include "Engine/GameObject/GameObject.h"

GraphNode_SetGameObjectTransform::GraphNode_SetGameObjectTransform()
{
	CreatePin("IN", PinDirection::PinDirection_IN, DataType::Exec);

	CreatePin("UUID", PinDirection::PinDirection_IN, DataType::Int);

	CreatePin("PosX", PinDirection::PinDirection_IN, DataType::Float);
	CreatePin("PosY", PinDirection::PinDirection_IN, DataType::Float);
	CreatePin("PosZ", PinDirection::PinDirection_IN, DataType::Float);

	CreatePin("RotX", PinDirection::PinDirection_IN, DataType::Float);
	CreatePin("RotY", PinDirection::PinDirection_IN, DataType::Float);
	CreatePin("RotZ", PinDirection::PinDirection_IN, DataType::Float);

	CreatePin("ScaleX", PinDirection::PinDirection_IN, DataType::Float);
	CreatePin("ScaleY", PinDirection::PinDirection_IN, DataType::Float);
	CreatePin("ScaleZ", PinDirection::PinDirection_IN, DataType::Float);

	CreatePin("OUT", PinDirection::PinDirection_OUT, DataType::Exec);

}

int GraphNode_SetGameObjectTransform::OnExec(class GraphNodeInstance* aNodeInstance)
{
	const int uuid = GetPinData<int>(aNodeInstance, 1);
	auto gameObject = GScene->FindGameObject(uuid);

	if (gameObject == nullptr) {
		LOG_WARNING(LogType::Components) << "Gaem Object NUll. GraphNode_SetGameObjectTransform.cpp";
		return -1;
	}

	Vec3f pos;
	Vec3f rot;
	Vec3f scale;
	
	pos.x = GetPinData<float>(aNodeInstance, 2);
	pos.y = GetPinData<float>(aNodeInstance, 3);
	pos.z = GetPinData<float>(aNodeInstance, 4);

	rot.x = GetPinData<float>(aNodeInstance, 5);
	rot.y = GetPinData<float>(aNodeInstance, 6);
	rot.z = GetPinData<float>(aNodeInstance, 7);

	scale.x = GetPinData<float>(aNodeInstance, 8);
	scale.y = GetPinData<float>(aNodeInstance, 9);
	scale.z = GetPinData<float>(aNodeInstance, 10);

	CU::Quaternion qaut(rot);

	gameObject->GetTransform().SetPosition(pos);
	gameObject->GetTransform().SetRotation(rot);
	gameObject->GetTransform().SetScale(scale);

	return 11;
}