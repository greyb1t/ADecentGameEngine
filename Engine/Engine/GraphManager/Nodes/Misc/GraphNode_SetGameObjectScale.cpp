#include "pch.h"
#include "Engine/Engine.h"
#include "Engine/Scene/Scene.h"
#include "GraphNode_SetGameObjectScale.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"
#include "Engine/GameObject/GameObject.h"

GraphNode_SetGameObjectScale::GraphNode_SetGameObjectScale()
{
	CreatePin("IN", PinDirection::PinDirection_IN, DataType::Exec);

	CreatePin("UUID", PinDirection::PinDirection_IN, DataType::Int);

	CreatePin("ScaleX", PinDirection::PinDirection_IN, DataType::Float);
	CreatePin("ScaleY", PinDirection::PinDirection_IN, DataType::Float);
	CreatePin("ScaleZ", PinDirection::PinDirection_IN, DataType::Float);

	CreatePin("OUT", PinDirection::PinDirection_OUT, DataType::Exec);

}

int GraphNode_SetGameObjectScale::OnExec(class GraphNodeInstance* aNodeInstance)
{
	const int uuid = GetPinData<int>(aNodeInstance, 1);
	auto gameObject = aNodeInstance->GetGameObject()->GetScene()->FindGameObject(uuid);

	if (gameObject == nullptr) {
		LOG_WARNING(LogType::Components) << "Gaem Object NUll. GraphNode_SetGameObjectScale.cpp";
		return -1;
	}

	Vec3f scale;
	
	scale.x = GetPinData<float>(aNodeInstance, 2);
	scale.y = GetPinData<float>(aNodeInstance, 3);
	scale.z = GetPinData<float>(aNodeInstance, 4);

	gameObject->GetTransform().SetScale(scale);

	return 5;
}