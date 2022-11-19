#include "pch.h"
#include "Engine/Engine.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneManager.h"
#include "Engine/GraphManager/GraphManager.h"
#include "Engine/Renderer/GraphicsEngine.h"
#include "GraphNode_QuitProgram.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"
#include "Engine/GameObject/GameObject.h"
#include <stdlib.h>

GraphNode_QuitProgram::GraphNode_QuitProgram()
{
	CreatePin("IN", PinDirection::PinDirection_IN, DataType::Exec);
}

int GraphNode_QuitProgram::OnExec(class GraphNodeInstance* aNodeInstance)
{
	exit(100);

	return -1;
}