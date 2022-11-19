#include "pch.h"
#include "GameObjectPrefabReferencesReflector.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/RecursiveGameObjectChildIterator.h"

Engine::GameObjectPrefabReferencesReflector::GameObjectPrefabReferencesReflector(
	GameObjectManager& aGameObjectManager,
	GameObject& aSourceGameObject)
	: mySourceGameObject(&aSourceGameObject),
	myGameObjectManager(&aGameObjectManager)
{
}

Engine::ReflectorResult Engine::GameObjectPrefabReferencesReflector::ReflectInternal(
	GameObjectRef& aValue,
	const std::string& aName,
	const ReflectionFlags aFlags)
{
	switch (aValue.myType)
	{
	case GameObjectRef::Type::SceneHierarchyReference:
	{
		auto g = myGameObjectManager->FindGameObjectShared(aValue.mySceneHierarchyReference.myUUID);
		aValue.myRef = g;

		if (g == nullptr)
		{
			LOG_WARNING(LogType::Engine) << "Unable to find scene gameobject reference of UUID: " << aValue.mySceneHierarchyReference.myUUID;
		}
	} break;

	case GameObjectRef::Type::PrefabHierarchyReference:
	{
		if (mySourceGameObject)
		{
			GameObject* topmostGameObject = mySourceGameObject;

			Transform* topmostParent = mySourceGameObject->GetTransform().GetTopMostParent();

			if (topmostParent)
			{
				topmostGameObject = topmostParent->GetGameObject();
			}

			bool found = false;

			if (topmostGameObject->GetPrefabGameObjectID() == aValue.myPrefabHierarchyReference.myPrefabGameObjectID)
			{
				aValue.myRef = topmostGameObject->GetTransform().GetGameObjectShared();
				found = true;
				break;
			}

			for (auto it = GameObjectChildIterator(*topmostGameObject); it != nullptr; it++)
			{
				if (it.DerefAsRaw()->GetPrefabGameObjectID() == aValue.myPrefabHierarchyReference.myPrefabGameObjectID)
				{
					aValue.myRef = it.DerefAsShared();
					found = true;
					break;
				}
			}

			if (!found)
			{
				LOG_WARNING(LogType::Engine) << "Unable to find prefab gameobject reference of PrefabGameObjectID: " << aValue.myPrefabHierarchyReference.myPrefabGameObjectID;
			}
		}
		else
		{
			LOG_ERROR(LogType::Engine) << "SourceGameObject is null, why? Ask filip";
		}
	} break;

	default:
		break;
	}

	return ReflectorResult_None;
}
