#include "pch.h"
#include "GraphNodeInstance.h"
#include "GraphNodeBase.h"
#include <assert.h>

#include "Engine/GraphManager/GraphManager.h"

GraphNodeInstance::GraphNodeInstance(GraphInstance* aGraphInstance, bool aCreateNewUID)
	: myUID(aCreateNewUID),
	myGraphInstance(aGraphInstance)
{
}

GraphNodeInstance::~GraphNodeInstance()
{
	myNodeType = nullptr;
}

bool IsOutput(std::vector<GraphNodePin>& pins, unsigned int aID)
{
	for (auto& pin : pins)
	{
		if (pin.Direction == PinDirection::PinDirection_OUT && pin.UID.ToUInt() == aID)
		{
			return true;
		}
	}
	return false;
}

void GraphNodeInstance::EnterStartNode(Payload& aPayload)
{
	assert(myNodeType->IsStartNode());

	int outputIndex = myNodeType->ExecWithPayload(this, aPayload);
	outputIndex = outputIndex > -1 ? myPins[outputIndex].UID.ToUInt() : -1;
	if (outputIndex > -1)
	{
		for (auto& link : myLinks)
		{
			if ((int)link.FromPinID == outputIndex && IsOutput(myPins, link.FromPinID))
			{
				link.Target->Enter();
			}
			else if (!IsOutput(myPins, link.FromPinID))
			{
				GraphManager::ShowFlow(myGraphInstance, link.LinkID);
			}
		}
	}
}

void GraphNodeInstance::Enter()
{
	int outputIndex = myNodeType->Exec(this);
	outputIndex = outputIndex > -1 ? myPins[outputIndex].UID.ToUInt() : -1;
	if (outputIndex > -1)
	{
		for (auto& link : myLinks)
		{
			if ((int)link.FromPinID == outputIndex && IsOutput(myPins, link.FromPinID))
			{
				link.Target->Enter();
			}
			else if (!IsOutput(myPins, link.FromPinID))
			{
				GraphManager::ShowFlow(myGraphInstance, link.LinkID);
			}
		}
	}
}

void GraphNodeInstance::ExitVia(unsigned int aPinIndex)
{
	GraphNodePin& pin = myPins[aPinIndex];
	std::vector< GraphNodeEdge*> links = GetLinksFromPin(pin.UID.ToUInt());
	for (auto link : links)
	{
		link->Target->Enter();
	}
}

void GraphNodeInstance::ConstructUniquePins()
{
	myPins = myNodeType->GetPins();
	for (auto& pin : myPins)
	{
		pin.Parent = this;
	}
}

bool GraphNodeInstance::CanAddLink(unsigned int aPinIdFromMe)
{
	GraphNodePin* pin = GetPinFromID(aPinIdFromMe);

	if (pin->Direction == PinDirection::PinDirection_IN && pin->DataType != DataType::Exec)
	{
		if (GetLinksFromPin(aPinIdFromMe).size() != 0)
		{
			return false;
		}
	}

	return true;
}

bool GraphNodeInstance::HasLinkBetween(unsigned int aFirstPin, unsigned int aSecondPin)
{
	for (auto link : myLinks)
	{
		if (link.FromPinID == aFirstPin && link.ToPinID == aSecondPin
			||
			link.ToPinID == aFirstPin && link.FromPinID == aSecondPin
			)
		{
			return true;
		}
	}
	return false;
}

bool GraphNodeInstance::AddLinkToVia(GraphNodeInstance* aLink, unsigned int aPinIdFromMe, unsigned int aPinIdToMe, unsigned int aLinkID)
{
	GraphNodePin* pin = GetPinFromID(aPinIdFromMe);

	if (pin->Direction == PinDirection::PinDirection_IN && pin->DataType != DataType::Exec)
	{
		if (GetLinksFromPin(aPinIdFromMe).size() != 0)
		{
			return false;
		}
	}


	myLinks.push_back(GraphNodeEdge(aLink, aPinIdFromMe, aPinIdToMe, aLinkID));
	return true;
}

void GraphNodeInstance::RemoveLinkToVia(GraphNodeInstance* aLink, unsigned int aPinThatIOwn)
{
	for (int i = 0; i < myLinks.size(); i++)
	{
		if (myLinks[i].Target == aLink)
		{
			if (myLinks[i].FromPinID == aPinThatIOwn)
			{
				myLinks.erase(myLinks.begin() + i);
				return;
			}
			else if (myLinks[i].ToPinID == aPinThatIOwn)
			{
				myLinks.erase(myLinks.begin() + i);
				return;
			}
		}
	}
	assert(0);
}

std::string GraphNodeInstance::WriteVariableType(const GraphNodePin& aPin) const
{
	if (aPin.DataType == DataType::Bool)
	{
		return "BOOL";
	}
	else if (aPin.DataType == DataType::Int)
	{
		return "INT";
	}
	else if (aPin.DataType == DataType::Float)
	{
		return "FLOAT";
	}
	else if (aPin.DataType == DataType::String)
	{
		return "STRING";
	}
	return "";
}

bool GraphNodeInstance::IsPinConnected(GraphNodePin& aPin)
{
	return GetLinksFromPin(aPin.UID.ToUInt()).size() > 0;
}

void GraphNodeInstance::ChangePinTypes(DataType aType)
{
	for (auto& pin : myPins)
	{
		if (pin.DataType != DataType::Exec)
			pin.DataType = aType;
	}
}

std::vector< GraphNodeEdge*> GraphNodeInstance::GetLinksFromPin(unsigned int aPinToFetchFrom)
{
	std::vector< GraphNodeEdge*> links;
	for (int i = 0; i < myLinks.size(); i++)
	{
		if (myLinks[i].FromPinID == aPinToFetchFrom)
		{
			links.push_back(&myLinks[i]);
		}
		else if (myLinks[i].ToPinID == aPinToFetchFrom)
		{
			links.push_back(&myLinks[i]);
		}
	}
	return links;

}

void GraphNodeInstance::SetVariable(DataType someType, DataPtr aDataPtr)
{
	myVariable->Set(someType, aDataPtr);
}

void GraphNodeInstance::SetVariableRaw(const char* someData, const size_t someSize)
{
	myVariable->SetRaw(someData, someSize);
}

GraphNodePin* GraphNodeInstance::GetPinFromID(unsigned int aUID)
{
	for (auto& pin : myPins)
	{
		if (pin.UID.ToUInt() == aUID)
		{
			return &pin;
		}
	}
	return nullptr;
}

int GraphNodeInstance::GetPinIndexFromPinUID(unsigned int aPinUID)
{
	for (int i = 0; i < myPins.size(); i++)
	{
		if (myPins[i].UID.ToUInt() == aPinUID)
		{
			return i;
		}
	}
	return -1;
}

void GraphNodeInstance::DebugUpdate()
{
	if (myNodeType)
	{
		myNodeType->DebugUpdate(this);
	}
}

void GraphNodeInstance::VisualUpdate(float aTimeDelta)
{
	myEnteredTimer -= aTimeDelta;
	if (myEnteredTimer <= 0.0f)
	{
		myEnteredTimer = 0.0f;
	}
}

bool GraphNodeInstance::FetchData(unsigned int aPinIndex, DataPtr& aResult)
{
	DataType outType;
	size_t outDataSize;

	FetchDataRaw(outType, aResult, outDataSize, aPinIndex);

	return outDataSize > 0;
}

GameObject* GraphNodeInstance::GetGameObject()
{
	return myGraphInstance->myGameObject;
}

void GraphNodeInstance::FetchDataRaw(DataType& outType, DataPtr& someData, size_t& outSize, unsigned int aPinToFetchFrom)
{
	// If we dont have any data, but or link might have it, the link pin might have data written to it as well, then return that
	if (!myNodeType->IsFlowNode())
	{
		if (myPins[aPinToFetchFrom].Direction == PinDirection::PinDirection_IN)
		{
			std::vector< GraphNodeEdge*> links = GetLinksFromPin(myPins[aPinToFetchFrom].UID.ToUInt());
			if (links.size() > 0)
			{
				// Get data from first link, wierd if we have more than two links to fetch data from
				int pinIndex = links[0]->Target->GetPinIndexFromPinUID(links[0]->ToPinID);
				if (pinIndex == -1)
				{
					assert(0);
				}

				GraphManager::ShowFlow(myGraphInstance, links[0]->LinkID);
				links[0]->Target->FetchDataRaw(outType, someData, outSize, pinIndex);
				//we have a link in a node that is supposed only to store data, apparently this is connected aswell
				return;
			}
		}
		else
		{
			Enter();
		}


	}
	else
	{
		if (myPins[aPinToFetchFrom].Direction == PinDirection::PinDirection_IN)
		{
			std::vector< GraphNodeEdge*> links = GetLinksFromPin(myPins[aPinToFetchFrom].UID.ToUInt());
			if (links.size() > 0)
			{
				int indexInVector = -1;
				for (int i = 0; i < links[0]->Target->myPins.size(); i++)
				{
					indexInVector = i;
					if (links[0]->Target->myPins[i].UID.ToUInt() == links[0]->ToPinID) // links[0] always get the data from the first connection, more connections are illigal
					{
						break;
					}
				}

				assert(indexInVector != -1);
				links[0]->Target->FetchDataRaw(outType, someData, outSize, indexInVector);
				return;
			}
		}
	}

	const GraphNodePin& dataPin = myPins[aPinToFetchFrom];
	// Data found, so set the return variables.

	outSize = 0;

	if (dataPin.Data)
	{
		outSize = dataPin.Data.GetSize();
	}

	someData = dataPin.Data;
	outType = dataPin.DataType;
}

