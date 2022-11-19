#include "pch.h"
#include "Engine/Engine.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneManager.h"
#include "Engine/GraphManager/GraphManager.h"
#include "Engine/Renderer/GraphicsEngine.h"
#include "GraphNode_SetWindowedMode.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"
#include "Engine/GameObject/GameObject.h"

GraphNode_SetWindowedMode::GraphNode_SetWindowedMode()
{
	CreatePin("IN", PinDirection::PinDirection_IN, DataType::Exec);

	CreatePin("", PinDirection::PinDirection_IN, DataType::Bool);

	CreatePin("OUT", PinDirection::PinDirection_OUT, DataType::Exec);
}

int GraphNode_SetWindowedMode::OnExec(class GraphNodeInstance* aNodeInstance)
{
	bool value = GetPinData<bool>(aNodeInstance, 1);

	if (value == true)
	{
		GetEngine().GetGraphicsEngine().GetWindowHandler().SetWindowState(Engine::WindowState::Fullscreen);
	}
	else
	{
		GetEngine().GetGraphicsEngine().GetWindowHandler().SetWindowState(Engine::WindowState::Windowed);
	}
	return 2;
}