#include "pch.h"
#include "GraphNode_Variable.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"
#include "Engine/GraphManager/Nodes/Base/GraphVariable.h"

GraphNode_Set::GraphNode_Set()
{
	myHiddenFromMenu = true;
	CreatePin("Exec", PinDirection::PinDirection_IN);
	CreatePin("Out", PinDirection::PinDirection_OUT);
	CreatePin("Value", PinDirection::PinDirection_IN, DataType::Variable);
	CreatePin("Result", PinDirection::PinDirection_OUT, DataType::Variable);
}

int GraphNode_Set::OnExec(GraphNodeInstance* aNodeInstance)
{
	DataType outType;
	DataPtr outPinData;
	size_t outDataSize = 0;

	GetPinData(aNodeInstance, 2, outType, outPinData, outDataSize);

	aNodeInstance->SetVariable(outType, outPinData);

	SetPinDataRaw(aNodeInstance, 3, outPinData.GetPtr<char>(), outDataSize);

	return 1;
}

std::string GraphNode_Set::GetInstanceName(const GraphNodeInstance* aNodeInstance) const
{
	return std::string("Set ") + aNodeInstance->GetVariable()->GetName();
}

GraphNode_Get::GraphNode_Get()
{
	myHiddenFromMenu = true;
	CreatePin("Value", PinDirection::PinDirection_OUT, DataType::Variable);
}

int GraphNode_Get::OnExec(GraphNodeInstance* aNodeInstance)
{
	DataType outType;
	DataPtr outPinData;

	aNodeInstance->GetVariable()->GetRaw(outType, outPinData);

	SetPinDataRaw(aNodeInstance, 0, outPinData.GetPtr<char>(), outPinData.GetSize());

	return -1;
}

std::string GraphNode_Get::GetInstanceName(const GraphNodeInstance* aNodeInstance) const
{
	return std::string("Get ") + aNodeInstance->GetVariable()->GetName();
}
