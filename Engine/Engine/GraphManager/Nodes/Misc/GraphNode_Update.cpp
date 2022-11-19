#include "pch.h"
#include "GraphNode_Update.h"
#include <iostream>
#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"


GraphNode_Update::GraphNode_Update()
{
	CreatePin("", PinDirection::PinDirection_OUT);
	//CreatePin("Updating", PinDirection::PinDirection_IN, DataType::Bool);
}

int GraphNode_Update::OnExec(class GraphNodeInstance* aNodeInstance)
{
	//const bool shouldRepeat = GetPinData<bool>(aNodeInstance, 1);
	//aNodeInstance->SetShouldTriggerAgain(shouldRepeat);
	return 0;
}
