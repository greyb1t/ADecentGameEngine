#include "pch.h"
#include "GraphNodeReflector.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"
#include "Engine/GraphManager/Nodes/Event/NodeEventStructs.h"

Engine::ReflectorResult Engine::GraphNodeReflector::ReflectInternal(int& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	myGraphNodeBase->CreatePin(aName, myPinDirection, DataType::Int);

	DynamicPin dynamicPin;
	dynamicPin.myName = aName;
	dynamicPin.myDataType = DataType::Int;

	myDynamicPins.push_back(dynamicPin);

	return ReflectorResult::ReflectorResult_None;
}

Engine::ReflectorResult Engine::GraphNodeReflector::ReflectInternal(float& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	myGraphNodeBase->CreatePin(aName, myPinDirection, DataType::Float);

	DynamicPin dynamicPin;
	dynamicPin.myName = aName;
	dynamicPin.myDataType = DataType::Float;

	myDynamicPins.push_back(dynamicPin);

	return ReflectorResult::ReflectorResult_None;
}

Engine::ReflectorResult Engine::GraphNodeReflector::ReflectInternal(bool& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	myGraphNodeBase->CreatePin(aName, myPinDirection, DataType::Bool);

	DynamicPin dynamicPin;
	dynamicPin.myName = aName;
	dynamicPin.myDataType = DataType::Bool;

	myDynamicPins.push_back(dynamicPin);

	return ReflectorResult::ReflectorResult_None;
}

Engine::ReflectorResult Engine::GraphNodeReflector::ReflectInternal(std::string& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	myGraphNodeBase->CreatePin(aName, myPinDirection, DataType::String);

	DynamicPin dynamicPin;
	dynamicPin.myName = aName;
	dynamicPin.myDataType = DataType::String;

	myDynamicPins.push_back(dynamicPin);

	return ReflectorResult::ReflectorResult_None;
}
