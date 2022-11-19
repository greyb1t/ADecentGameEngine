#include "pch.h"
#include "Engine/Reflection/SetPinDataReflector.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"
#include "Engine/GraphManager/Nodes/Event/NodeEventStructs.h"

Engine::ReflectorResult Engine::SetPinDataReflector::ReflectInternal(int& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (myDynamicPin->myName == aName)
	{
		myGraphNodeBase->SetPinData(myGraphNodeInstance, myPinIndex, aValue);
	}
	return ReflectorResult::ReflectorResult_None;
}

Engine::ReflectorResult Engine::SetPinDataReflector::ReflectInternal(float& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (myDynamicPin->myName == aName)
	{
		myGraphNodeBase->SetPinData(myGraphNodeInstance, myPinIndex, aValue);
	}
	return ReflectorResult::ReflectorResult_None;
}

Engine::ReflectorResult Engine::SetPinDataReflector::ReflectInternal(bool& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (myDynamicPin->myName == aName)
	{
		myGraphNodeBase->SetPinData(myGraphNodeInstance, myPinIndex, aValue);
	}

	return ReflectorResult::ReflectorResult_None;
}

Engine::ReflectorResult Engine::SetPinDataReflector::ReflectInternal(std::string& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (myDynamicPin->myName == aName)
	{
		myGraphNodeBase->SetPinData(myGraphNodeInstance, myPinIndex, aValue);
	}
	return ReflectorResult::ReflectorResult_None;
}
