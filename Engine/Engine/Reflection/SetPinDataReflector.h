#pragma once
#include "Engine/Reflection/Reflector.h"

class GraphNodeBase;
class GraphNodeInstance;
struct DynamicPin;

namespace Engine {

	class SetPinDataReflector :
		public Reflector
	{
	public:
		SetPinDataReflector(GraphNodeBase* aGraphNodeBase, GraphNodeInstance* aGraphNodeInstance, const DynamicPin& aDynamicPin, int aPinIndex) :
			myGraphNodeBase(aGraphNodeBase), myGraphNodeInstance(aGraphNodeInstance), myDynamicPin(&aDynamicPin), myPinIndex(aPinIndex) {}


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
		GraphNodeInstance* myGraphNodeInstance;
		const DynamicPin* myDynamicPin;
		int myPinIndex;

	};
}

