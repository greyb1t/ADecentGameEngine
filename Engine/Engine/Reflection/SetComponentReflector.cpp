#include "pch.h"
#include "Engine/Reflection/SetComponentReflector.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"
#include "Engine/GraphManager/Nodes/Event/NodeEventStructs.h"

Engine::ReflectorResult Engine::SetComponentReflector::ReflectInternal(int& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (myDynamicPin->myName == aName)
	{
		aValue = myGraphNodeBase->GetPinData<int>(myGraphNodeInstance, myPinIndex);
	}
	return ReflectorResult::ReflectorResult_None;
}

Engine::ReflectorResult Engine::SetComponentReflector::ReflectInternal(float& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (myDynamicPin->myName == aName)
	{
		aValue = myGraphNodeBase->GetPinData<float>(myGraphNodeInstance, myPinIndex);
	}
	return ReflectorResult::ReflectorResult_None;
}

Engine::ReflectorResult Engine::SetComponentReflector::ReflectInternal(bool& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (myDynamicPin->myName == aName)
	{
		aValue = myGraphNodeBase->GetPinData<bool>(myGraphNodeInstance, myPinIndex);
	}

	return ReflectorResult::ReflectorResult_None;
}

Engine::ReflectorResult Engine::SetComponentReflector::ReflectInternal(std::string& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (myDynamicPin->myName == aName)
	{
		aValue = myGraphNodeBase->GetPinData<std::string>(myGraphNodeInstance, myPinIndex);
	}
	return ReflectorResult::ReflectorResult_None;
}
