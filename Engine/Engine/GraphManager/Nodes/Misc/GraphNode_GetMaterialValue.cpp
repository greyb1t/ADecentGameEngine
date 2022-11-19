#include "pch.h"
#include "Engine/Engine.h"
#include "Engine/Scene/Scene.h"
#include "GraphNode_GetMaterialValue.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"
#include "Engine/GameObject/Components/ModelComponent.h"
#include "Engine/GameObject/GameObject.h"

GraphNode_GetMaterialValue::GraphNode_GetMaterialValue()
{
	CreatePin("UUID", PinDirection::PinDirection_IN, DataType::Int);

	CreatePin("Variable Name", PinDirection::PinDirection_IN, DataType::String);

	CreatePin("X", PinDirection::PinDirection_OUT, DataType::Float);
	CreatePin("Y", PinDirection::PinDirection_OUT, DataType::Float);
	CreatePin("Z", PinDirection::PinDirection_OUT, DataType::Float);
	CreatePin("W", PinDirection::PinDirection_OUT, DataType::Float);
}

int GraphNode_GetMaterialValue::OnExec(class GraphNodeInstance* aNodeInstance)
{
	const int uuid = GetPinData<int>(aNodeInstance, 1);
	auto gameObject = aNodeInstance->GetGameObject()->GetScene()->FindGameObject(uuid);
	auto variableName = GetPinData<std::string>(aNodeInstance, 2);

	if (gameObject == nullptr) {
		LOG_WARNING(LogType::Components) << "Gaem Object NUll. GraphNode_GetMaterialValue.cpp";
		return -1;
	}

	auto materialValues = gameObject->GetComponent<Engine::ModelComponent>()->GetMeshInstances()[2].GetMaterialInstance().GetFloat4(variableName);
	Vec4f value;
	
	SetPinData<float>(aNodeInstance, 2, materialValues.x);
	SetPinData<float>(aNodeInstance, 3, materialValues.y);
	SetPinData<float>(aNodeInstance, 4, materialValues.z);
	SetPinData<float>(aNodeInstance, 5, materialValues.w);
	
	return -1;
}