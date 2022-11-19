#include "pch.h"
#include "Engine/Engine.h"
#include "Engine/Scene/Scene.h"
#include "GraphNode_SetGameObjectRotation.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"
#include "Engine/GameObject/GameObject.h"

GraphNode_SetGameObjectRotation::GraphNode_SetGameObjectRotation()
{
	CreatePin("IN", PinDirection::PinDirection_IN, DataType::Exec);

	CreatePin("UUID", PinDirection::PinDirection_IN, DataType::Int);

	CreatePin("RotX", PinDirection::PinDirection_IN, DataType::Float);
	CreatePin("RotY", PinDirection::PinDirection_IN, DataType::Float);
	CreatePin("RotZ", PinDirection::PinDirection_IN, DataType::Float);

	CreatePin("OUT", PinDirection::PinDirection_OUT, DataType::Exec);

}

int GraphNode_SetGameObjectRotation::OnExec(class GraphNodeInstance* aNodeInstance)
{
	const int uuid = GetPinData<int>(aNodeInstance, 1);
	auto gameObject = aNodeInstance->GetGameObject()->GetScene()->FindGameObject(uuid);

	if (gameObject == nullptr) {
		LOG_WARNING(LogType::Components) << "Gaem Object NUll. GraphNode_SetGameObjectRotation.cpp";
		return -1;
	}

	Vec3f rot;
	
	rot.x = GetPinData<float>(aNodeInstance, 2);
	rot.y = GetPinData<float>(aNodeInstance, 3);
	rot.z = GetPinData<float>(aNodeInstance, 4);

	Quatf qaut(rot);

	gameObject->GetTransform().SetRotation(qaut);

	return 5;
}