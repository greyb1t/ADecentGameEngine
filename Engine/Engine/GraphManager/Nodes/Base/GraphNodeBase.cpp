#include "pch.h"
#include "GraphNodeBase.h"
#include "Engine/GraphManager/Nodes/Misc/GraphNode_Print.h"
#include "Engine/GraphManager/Nodes/Misc/GraphNode_Update.h"

#include "GraphNodeInstance.h"

#include "Engine/GraphManager/Nodes/Math/GraphNode_MathAdd.h"

//std::vector<unsigned int> UID::myAllUIDs;
unsigned int UID::myGlobalUID = 0;


int GraphNodeBase::OnExec(GraphNodeInstance*)
{
	return -1;
}

int GraphNodeBase::OnExecWithPayload(GraphNodeInstance* aNodeInstance, Payload& aPayload)
{
	aNodeInstance;
	aPayload;
	assert(IsStartNode());
	return -1;
}

GraphNodeBase::~GraphNodeBase()
{
	myPins.clear();
}

int GraphNodeBase::Exec(GraphNodeInstance* prevNode)
{
	const int index = OnExec(prevNode);
	prevNode->myEnteredTimer = 3.0f;
	return index;
}

int GraphNodeBase::ExecWithPayload(GraphNodeInstance* prevNode, Payload& aPayload)
{
	const int index = OnExecWithPayload(prevNode, aPayload);
	prevNode->myEnteredTimer = 3.0f;
	return index;
}

bool GraphNodeBase::IsFlowNode()
{
	for (const auto& pin : myPins)
	{
		if (pin.DataType == DataType::Exec)
		{
			return true;
		}
	}
	return false;
}

void GraphNodeBase::CreatePin(std::string aLabel, PinDirection aDirection, DataType aPinType, bool isReadOnly, bool allowConstructInPlace)
{
	myPins.push_back({ aLabel, aDirection, aPinType, allowConstructInPlace });

	myPins[ myPins.size() - 1 ].IsReadOnly = isReadOnly;
}

void GraphNodeBase::GetPinData(GraphNodeInstance* aNode, unsigned int aPinIndex, DataType& outType, DataPtr& someData, size_t& outSize) const
{
	aNode->FetchDataRaw(outType, someData, outSize, aPinIndex);
}

void GraphNodeBase::SetPinDataRaw(GraphNodeInstance* aNodeInstance, unsigned aPinIndex, const char* someRawData,
	size_t someDataSize)
{
	GraphNodePin& pin = aNodeInstance->myPins[aPinIndex];
	if(!pin.Data)
	{
		pin.Data.Ptr = DBG_NEW char[someDataSize]();
		pin.Data.Size = someDataSize;
		pin.Data.Type = pin.DataType;
	}

	memcpy(pin.Data.Ptr, someRawData, someDataSize);
}
