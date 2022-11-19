#pragma once
#include "Engine/Reflection/Reflector.h"
#include "Engine/GraphManager/Nodes/Base/NodeTypes.h"

class GraphNodeBase;
struct DynamicPin;

namespace Engine
{
	class GraphNodeReflector :
		public Reflector
	{
	public:
		GraphNodeReflector(GraphNodeBase* aGraphNodeBase, std::vector<DynamicPin>& aDynamicPins, PinDirection aPinDirection) :
			myGraphNodeBase(aGraphNodeBase), myDynamicPins(aDynamicPins), myPinDirection(aPinDirection) {}


		virtual ReflectorResult ReflectInternal(int& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None);
		virtual ReflectorResult ReflectInternal(float& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None);
		virtual ReflectorResult ReflectInternal(bool& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None);
		virtual ReflectorResult ReflectInternal(std::string& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None);
	private:
		GraphNodeBase* myGraphNodeBase;
		std::vector<DynamicPin>& myDynamicPins;
		PinDirection myPinDirection;
	};
}

