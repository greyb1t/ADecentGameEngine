#include "pch.h"
#include "FindValueReflector.h"

Engine::FindValueReflector::FindValueReflector(
	std::function<ReflectorResult(void*)> aFoundValueCallback,
	const std::string& aValueName,
	const int aTargetDepth,
	const int aTargetCounter)
	: myFoundValueCallback(aFoundValueCallback),
	myValueName(aValueName),
	myTargetDepth(aTargetDepth),
	myTargetCounter(aTargetCounter)
{
}

Engine::ReflectorResult Engine::FindValueReflector::ReflectInternal(Reflectable& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	//if (GetDepth() == myTargetDepth && myValueName == aName)
	//{
	//	return myFoundValueCallback(&aValue);
	//}

	aValue.Reflect(*this);

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::FindValueReflector::ReflectInternal(GameObjectRef& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (myTargetCounter == GetCounter() && GetDepth() == myTargetDepth && myValueName == aName)
	{
		return myFoundValueCallback(&aValue);
	}

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::FindValueReflector::ReflectInternal(uint64_t& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (myTargetCounter == GetCounter() && GetDepth() == myTargetDepth && myValueName == aName)
	{
		return myFoundValueCallback(&aValue);
	}

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::FindValueReflector::ReflectInternal(Enum& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (myTargetCounter == GetCounter() && GetDepth() == myTargetDepth && myValueName == aName)
	{
		return myFoundValueCallback(&aValue);
	}

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::FindValueReflector::ReflectInternal(Transform& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (myTargetCounter == GetCounter() && GetDepth() == myTargetDepth && myValueName == aName)
	{
		return myFoundValueCallback(&aValue);
	}

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::FindValueReflector::ReflectInternal(GameObjectPrefabRef& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (myTargetCounter == GetCounter() && GetDepth() == myTargetDepth && myValueName == aName)
	{
		return myFoundValueCallback(&aValue);
	}

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::FindValueReflector::ReflectInternal(VFXRef& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (myTargetCounter == GetCounter() && GetDepth() == myTargetDepth && myValueName == aName)
	{
		return myFoundValueCallback(&aValue);
	}

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::FindValueReflector::ReflectInternal(AnimationCurveRef& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (myTargetCounter == GetCounter() && GetDepth() == myTargetDepth && myValueName == aName)
	{
		return myFoundValueCallback(&aValue);
	}

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::FindValueReflector::ReflectInternal(VisualGraphPair& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (myTargetCounter == GetCounter() && GetDepth() == myTargetDepth && myValueName == aName)
	{
		return myFoundValueCallback(&aValue);
	}

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::FindValueReflector::ReflectInternal(TextureRef& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (myTargetCounter == GetCounter() && GetDepth() == myTargetDepth && myValueName == aName)
	{
		return myFoundValueCallback(&aValue);
	}

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::FindValueReflector::ReflectInternal(AnimationPair& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (myTargetCounter == GetCounter() && GetDepth() == myTargetDepth && myValueName == aName)
	{
		return myFoundValueCallback(&aValue);
	}

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::FindValueReflector::ReflectInternal(MaterialRef& aValue, const MaterialType aType, const std::string& aName, const ReflectionFlags aFlags)
{
	if (myTargetCounter == GetCounter() && GetDepth() == myTargetDepth && myValueName == aName)
	{
		return myFoundValueCallback(&aValue);
	}

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::FindValueReflector::ReflectInternal(ModelRef& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (myTargetCounter == GetCounter() && GetDepth() == myTargetDepth && myValueName == aName)
	{
		return myFoundValueCallback(&aValue);
	}

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::FindValueReflector::ReflectInternal(std::string& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (myTargetCounter == GetCounter() && GetDepth() == myTargetDepth && myValueName == aName)
	{
		return myFoundValueCallback(&aValue);
	}

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::FindValueReflector::ReflectInternal(bool& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (myTargetCounter == GetCounter() && GetDepth() == myTargetDepth && myValueName == aName)
	{
		return myFoundValueCallback(&aValue);
	}

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::FindValueReflector::ReflectInternal(Vec4f& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (myTargetCounter == GetCounter() && GetDepth() == myTargetDepth && myValueName == aName)
	{
		return myFoundValueCallback(&aValue);
	}

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::FindValueReflector::ReflectInternal(Vec3f& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (myTargetCounter == GetCounter() && GetDepth() == myTargetDepth && myValueName == aName)
	{
		return myFoundValueCallback(&aValue);
	}

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::FindValueReflector::ReflectInternal(Vec2f& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (myTargetCounter == GetCounter() && GetDepth() == myTargetDepth && myValueName == aName)
	{
		return myFoundValueCallback(&aValue);
	}

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::FindValueReflector::ReflectInternal(float& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (myTargetCounter == GetCounter() && GetDepth() == myTargetDepth && myValueName == aName)
	{
		return myFoundValueCallback(&aValue);
	}

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::FindValueReflector::ReflectInternal(int& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (myTargetCounter == GetCounter() && GetDepth() == myTargetDepth && myValueName == aName)
	{
		return myFoundValueCallback(&aValue);
	}

	return ReflectorResult_None;
}