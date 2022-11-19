#include "pch.h"
#include "GameObjectRef.h"

bool Engine::GameObjectRef::operator==(const GameObjectRef& aOther) const
{
	return myType == aOther.myType && myPrefabHierarchyReference.myPrefabGameObjectID == aOther.myPrefabHierarchyReference.myPrefabGameObjectID;
}

bool Engine::GameObjectRef::operator!=(const GameObjectRef& aOther) const
{
	return !(*this == aOther);
}

Engine::GameObjectRef::GameObjectRef()
{
}

bool Engine::GameObjectRef::IsValid() const
{
	return !myRef.expired();
}

GameObject* Engine::GameObjectRef::Get()
{
	return myRef.lock().get();
}

const GameObject* Engine::GameObjectRef::Get() const
{
	return myRef.lock().get();
}

Engine::GameObjectRef::operator bool() const
{
	return !myRef.expired();
}

const char* Engine::GameObjectRef::TypeToString(const Type aType)
{
	switch (aType)
	{
	case Type::SceneHierarchyReference: return "SceneHierarchyReference";
	case Type::PrefabHierarchyReference: return "PrefabHierarchyReference";
	case Type::Unknown:
		break;
	default:
		break;
	}

	return "Unknown";
}
