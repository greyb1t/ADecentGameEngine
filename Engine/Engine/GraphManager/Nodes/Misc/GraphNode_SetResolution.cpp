#include "pch.h"
#include "Engine/Engine.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneManager.h"
#include "Engine/GraphManager/GraphManager.h"
#include "Engine/Renderer/GraphicsEngine.h"
#include "GraphNode_SetResolution.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"
#include "Engine/GameObject/GameObject.h"

GraphNode_SetResolution::GraphNode_SetResolution()
{
	CreatePin("IN", PinDirection::PinDirection_IN, DataType::Exec);

	CreatePin("Width", PinDirection::PinDirection_IN, DataType::Int);
	CreatePin("Height", PinDirection::PinDirection_IN, DataType::Int);

	CreatePin("OUT", PinDirection::PinDirection_OUT, DataType::Exec);
}

int GraphNode_SetResolution::OnExec(class GraphNodeInstance* aNodeInstance)
{
	int width = GetPinData<int>(aNodeInstance, 1);
	int height = GetPinData<int>(aNodeInstance, 2);

	Vec2ui res;
	res.x = width;
	res.y = height;

	GetEngine().GetGraphicsEngine().GetWindowHandler().SetWindowResolution(res);
	//GetEngine().GetGraphicsEngine().GetWindowHandler().SetWindowState(Renderer::WindowState::Fullscreen);

	return 3;
}