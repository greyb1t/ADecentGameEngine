#include "pch.h"
#include "Engine/Engine.h"
#include "Engine/Scene/Scene.h"
#include "GraphNode_SetMaterialValue.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"
#include "Engine/GameObject/Components/ModelComponent.h"
#include "Engine/GameObject/GameObject.h"

GraphNode_SetMaterialValue::GraphNode_SetMaterialValue()
{
	CreatePin("IN", PinDirection::PinDirection_IN, DataType::Exec);

	CreatePin("UUID", PinDirection::PinDirection_IN, DataType::Int);

	CreatePin("Variable Name", PinDirection::PinDirection_IN, DataType::String);

	CreatePin("X", PinDirection::PinDirection_IN, DataType::Float);
	CreatePin("Y", PinDirection::PinDirection_IN, DataType::Float);
	CreatePin("Z", PinDirection::PinDirection_IN, DataType::Float);
	CreatePin("W", PinDirection::PinDirection_IN, DataType::Float);

	CreatePin("OUT", PinDirection::PinDirection_OUT, DataType::Exec);
}

int GraphNode_SetMaterialValue::OnExec(class GraphNodeInstance* aNodeInstance)
{
	const int uuid = GetPinData<int>(aNodeInstance, 1);
	auto gameObject = aNodeInstance->GetGameObject()->GetScene()->FindGameObject(uuid);
	auto variableName = GetPinData<std::string>(aNodeInstance, 2);

	if (gameObject == nullptr) {
		LOG_WARNING(LogType::Components) << "Gaem Object NUll. GraphNode_SetMaterialValue.cpp";
		return -1;
	}

	Vec4f value;
	
	value.x = GetPinData<float>(aNodeInstance, 2);
	value.y = GetPinData<float>(aNodeInstance, 3);
	value.z = GetPinData<float>(aNodeInstance, 4);
	value.w = GetPinData<float>(aNodeInstance, 5);

	auto materialValues = gameObject->GetComponent<Engine::ModelComponent>()->GetMeshInstances()[2].GetMaterialInstance().GetFloat4(variableName);

	gameObject->GetComponent<Engine::ModelComponent>()->GetMeshInstances()[2].GetMaterialInstance().SetFloat4(variableName, value);

	return 5;
}