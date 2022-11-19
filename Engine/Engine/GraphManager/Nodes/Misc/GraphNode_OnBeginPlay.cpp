#include "pch.h"
#include "GraphNode_OnBeginPlay.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"

GraphNode_OnBeginPlay::GraphNode_OnBeginPlay()
{
	CreatePin("GameObject ID", PinDirection::PinDirection_OUT, DataType::Int);

	CreatePin("OUT", PinDirection::PinDirection_OUT, DataType::Exec);
}

int GraphNode_OnBeginPlay::OnExecWithPayload(class GraphNodeInstance* aNodeInstance, Payload& aPayload)
{
	const int gameObjectId = aPayload.GetEntry<int>(0);

	SetPinData(aNodeInstance, 0, gameObjectId);

	return 1;
}

