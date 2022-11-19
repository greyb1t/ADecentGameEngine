#include "pch.h"
#include "GameObject.h"
#include "../Engine.h"
#include "Engine/Reflection/Reflector.h"
#include "RecursiveGameObjectChildIterator.h"
#include "Prefab/GameObjectPrefabInstance.h"
#include "Prefab/GameObjectPrefab.h"
#include "Engine/ResourceManagement/Resources/GameObjectPrefabResource.h"
#include "Engine/Reflection/GameObjectPrefabReflector.h"
#include "Engine/Editor/Editor.h"

GameObject::GameObject()
{
}

void GameObject::InitTransform(const Shared<GameObject>& aThisGameObjectAsShared)
{
	myTransform.myGameObject = aThisGameObjectAsShared;
}

GameObject::~GameObject()
{
	if (!myScene)
	{
		return;
	}

	for (int i = static_cast<int>(myComponents.size()) - 1; i >= 0; i--)
	{
		myScene->GetComponentSystem().ReleaseComponent(myComponents[i]);
		myComponents[i] = nullptr;
	}
}

void GameObject::InternalAwake()
{
	assert(myHasAwaked == false && "has already started");

	myHasAwaked = true;

	Awake();

	for (size_t i = 0; i < myComponents.size(); i++)
	{
		if (!myComponents[i]->IsActive())
		{
			continue;
		}

		if (myComponents[i]->myHasAwaked)
		{
			return;
		}

		myComponents[i]->myHasAwaked = true;

		myComponents[i]->Awake();
	}
}

void GameObject::InternalStart()
{
	assert(myHasStarted == false && "has already started");

	myHasStarted = true;

	Start();

	/*
	// Must be Int loop becuse we add component in Start()
	for (size_t i = 0; i < myComponents.size(); i++)
	{
		if (!myComponents[i]->IsActive())
		{
			continue;
		}

		myComponents[i]->Awake();
	}
	*/


	for (size_t i = 0; i < myComponents.size(); i++)
	{
		if (!myComponents[i]->IsActive())
		{
			continue;
		}

		if (myComponents[i]->myHasStarted)
		{
			return;
		}

		myComponents[i]->myHasStarted = true;

		myComponents[i]->Start();
	}
}

Weak<GameObject> GameObject::GetWeak() const
{
	return myWeakPtr;
}

Component* GameObject::GetComponentByTypeId(const int aTypeId)
{
	for (int i = 0; i < myComponents.size(); i++)
	{
		if (myComponents[i]->GetComponentTypeID() == aTypeId)
		{
			return myComponents[i];
		}
	}

	return nullptr;
}

Engine::UUID GameObject::GetUUID() const
{
	return myUUID;
}

Engine::Scene* GameObject::GetScene() const
{
	return myScene;
}

void GameObject::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myUUID.GetRef(), ourReflectUUIDStr);
	aReflector.Reflect(myIsActive, ourReflectIsActiveStr);
	aReflector.Reflect(myName, ourReflectNameStr);
	aReflector.Reflect(myIsFromUnreal, ourReflectIsFromUnrealStr);
	aReflector.Reflect(myTransform, ourReflectTransformStr);

	eTagClass tagClass(static_cast<int>(myTag));
	aReflector.Reflect(tagClass, ourReflectTagStr);
	myTag = static_cast<eTag>(tagClass.GetValue());

	uint64_t layerMaskInt = static_cast<uint64_t>(myLayer);
	aReflector.Reflect(layerMaskInt, ourReflectLayerMask);
	myLayer = layerMaskInt;

	uint64_t physicalLayerMaskInt = static_cast<uint64_t>(myPhysicalLayerMask);
	aReflector.Reflect(physicalLayerMaskInt, ourReflectPhysicalLayerMask);
	myPhysicalLayerMask = physicalLayerMaskInt;

	uint64_t detectionLayerInt = static_cast<uint64_t>(myDetectionLayerMask);
	aReflector.Reflect(detectionLayerInt, ourReflectDetectionLayerMask);
	myDetectionLayerMask = detectionLayerInt;

	ReflectLayerImgui(aReflector, "Layers", myLayer);
	ReflectLayerImgui(aReflector, "Physical Layers", myPhysicalLayerMask);
	ReflectLayerImgui(aReflector, "Detection Layers", myDetectionLayerMask);

	// Even tho we reflect this in ReflectPrefab() we reflect 
	// this here to be able to save it and not ruin other reflection stuff
	aReflector.SetNextItemHidden();
	aReflector.Reflect(myPrefab, ourReflectPrefabStr);
	aReflector.SetNextItemHidden();
	aReflector.Reflect(myPrefabGameObjectID, ourReflectPrefabGameObjectIDStr);
}

bool GameObject::ReflectPrefab(
	Engine::Reflector& aReflector,
	const std::unordered_map<std::string, Shared<Engine::GameObjectPrefab>>& aPrefabGameObjectStates)
{
	if (aReflector.Reflect(
		myPrefab,
		ourReflectPrefabStr,
		Engine::ReflectionFlags::ReflectionFlags_None) & Engine::ReflectorResult_Changed)
	{
		if (!myPrefab || !myPrefab->IsValid())
		{
			return false;
		}

		// If we came here, this gameobject is a top level prefab instance
		auto findResult = aPrefabGameObjectStates.find(myPrefab->GetPath());

		if (findResult != aPrefabGameObjectStates.end())
		{
			// If found, sync the prefab and compare against this saved gameobject prefab state

			assert(myPrefab && myPrefab->IsValid());

			GameObject* prefabGameObject = myPrefab->Get().GetGameObject().get();
			GameObject* unchangedPrefabGameObject = findResult->second->GetGameObject().get();

			myPrefabGameObject = prefabGameObject;

			auto gameObjectIt = Engine::GameObjectChildIterator(*this);

			auto unchangedPrefabGameObjectIt = Engine::GameObjectChildIterator(*unchangedPrefabGameObject);

			auto changedPrefabGameObjectIt = Engine::GameObjectChildIterator(*prefabGameObject);

			for (;
				gameObjectIt != nullptr &&
				unchangedPrefabGameObjectIt != nullptr &&
				changedPrefabGameObjectIt != nullptr;
				gameObjectIt++,
				unchangedPrefabGameObjectIt++,
				changedPrefabGameObjectIt++)
			{
				// Does not matter if unchangedPrefabGameObjectIt or changedPrefabGameObjectIt, both are same
				// EDIT: not anymore, now it matters
				gameObjectIt.DerefAsRaw()->myPrefabGameObject = changedPrefabGameObjectIt.DerefAsRaw();
			}

			SyncWithPrefabFull(unchangedPrefabGameObject, prefabGameObject);
		}
	}

	return true;
}

void GameObject::ReflectChildrenAndComponents(Engine::Reflector& aReflector)
{
	Reflect(aReflector);

	for (Component* c : myComponents)
	{
		c->Reflect(aReflector);
	}

	for (auto it = Engine::GameObjectChildIterator(*this); it != nullptr; it++)
	{
		it.DerefAsRaw()->Reflect(aReflector);

		for (Component* c : it.DerefAsRaw()->myComponents)
		{
			c->Reflect(aReflector);
		}
	}
}

void GameObject::SetIsFromUnreal(const bool aIsFromUnreal)
{
	myIsFromUnreal = aIsFromUnreal;
}

bool GameObject::IsFromUnreal() const
{
	return myIsFromUnreal;
}

Shared<GameObject> GameObject::Clone(
	Engine::GameObjectManager& aGameObjectManager,
	const bool aGenerateNewUUID,
	const bool aRecursively)
{
	return CloneInternal(aGameObjectManager, nullptr, aGenerateNewUUID, false, aRecursively);
}

Shared<GameObject> GameObject::CloneAsPrefabInstance(
	Engine::GameObjectManager& aGameObjectManager,
	const bool aGenerateNewUUID)
{
	Shared<GameObject> g = CloneInternal(aGameObjectManager, nullptr, aGenerateNewUUID, true, true);

	return g;
}

Shared<GameObject> GameObject::CloneInternal(
	Engine::GameObjectManager& aGameObjectManager,
	GameObject* aParent,
	const bool aGenerateNewUUID,
	const bool aIsPrefabInstance,
	const bool aRecursively)
{
	Shared<GameObject> g;

	//if (aIsPrefabInstance)
	//{
	//	g = MakeShared<GameObject>(*this);
	//}
	//else
	//{
	g = MakeShared<GameObject>(*this);
	// }

	g->myComponents.clear();

	for (Component* c : myComponents)
	{
		// TODO: if we ever pool the components, we cannot new them in Clone() like this
		if (auto newComponent = c->Clone())
		{
			newComponent->ChangeParentGameObject(*g);

			g->myComponents.push_back(newComponent);
		}
	}

	g->GetTransform().myChildren.clear();
	g->GetTransform().myParent = nullptr;

	if (aParent != nullptr)
	{
		g->GetTransform().myParent = &aParent->GetTransform();
		g->GetTransform().myParent->AddChild(&g->GetTransform());
	}

	g->myWeakPtr = g;

	if (aGenerateNewUUID)
	{
		//g->myUUID = Engine::GameObjectManager::ourGameObjectIndex++;
		g->myUUID = Engine::UUID();
	}

	g->myScene = aGameObjectManager.myScene;

	g->GetTransform().myGameObject = g;

	// Adds it to the gameobject manager directly
	// Why, because we cannot do it later because it creates Shared<GameObject>
	// that goes out of scope if nothing owns it
	{
		for (Component* c : g->myComponents)
		{
			aGameObjectManager.GetComponentSystem().AddExistingComponent(c);
		}
		aGameObjectManager.myGameObjectsToAdd.push_back(g);
	}

	if (aRecursively)
	{
		for (auto& children : GetTransform().GetChildren())
		{
			assert(children->GetGameObject() && "why is this not valid");

			children->GetGameObject()->CloneInternal(
				aGameObjectManager,
				g.get(),
				aGenerateNewUUID,
				aIsPrefabInstance,
				aRecursively);
		}
	}

	return g;
}

bool GameObject::IsChildOf(GameObject& aGameObject)
{
	auto it = Engine::GameObjectChildIterator(aGameObject);

	for (; it != nullptr; it++)
	{
		std::string name = it.DerefAsRaw()->GetName();

		if (it.DerefAsRaw()->GetTransform().GetGameObject() == this)
		{
			return true;
		}
	}

	return false;

	/*
	ForEachChild([&aGameObject](const Shared<const GameObject>& g)
		{
			if (g->GetTransform().GetGameObject() == &aGameObject)
			{
				return true;
			}
		});
	*/

	return false;
}

bool GameObject::IsPrefabInstance() const
{
	if (myPrefabGameObject != nullptr)
	{
		return true;
	}

	return false;
}

const GameObjectPrefabRef& GameObject::GetPrefab() const
{
	return myPrefab;
}

GameObject* GameObject::GetPrefabGameObject()
{
	return myPrefabGameObject;
}

void GameObject::SyncWithPrefab(
	GameObject* aUnchangedPrefabGameObject,
	GameObject* aChangedPrefabGameObject)
{
	// Update the top gameobject values
	Engine::GameObjectPrefabReflector refl1(*this, aUnchangedPrefabGameObject);
	// prefabGameObject->Reflect(refl1);
	refl1.Reflect(aChangedPrefabGameObject->myIsActive, ourReflectIsActiveStr);
	refl1.Reflect(aChangedPrefabGameObject->myName, ourReflectNameStr);

	// NOTE(filip): removed atm due to syncing the hierarchy or something like that
	refl1.Reflect(aChangedPrefabGameObject->myTransform, ourReflectTransformStr);

	eTagClass tagClass(static_cast<int>(aChangedPrefabGameObject->myTag));
	refl1.Reflect(tagClass, ourReflectTagStr);
	aChangedPrefabGameObject->myTag = static_cast<eTag>(tagClass.GetValue());

	uint64_t layerMaskInt = static_cast<uint64_t>(aChangedPrefabGameObject->myLayer);
	refl1.Reflect(layerMaskInt, ourReflectLayerMask);
	aChangedPrefabGameObject->myLayer = layerMaskInt;

	uint64_t physicalLayerMaskInt = static_cast<uint64_t>(aChangedPrefabGameObject->myPhysicalLayerMask);
	refl1.Reflect(physicalLayerMaskInt, ourReflectPhysicalLayerMask);
	aChangedPrefabGameObject->myPhysicalLayerMask = physicalLayerMaskInt;

	uint64_t detectionLayerInt = static_cast<uint64_t>(aChangedPrefabGameObject->myDetectionLayerMask);
	refl1.Reflect(detectionLayerInt, ourReflectDetectionLayerMask);
	aChangedPrefabGameObject->myDetectionLayerMask = detectionLayerInt;

	// If have multiple of same component, give error?

	// Sync the component values
	for (Component* changedPrefabComponent : aChangedPrefabGameObject->myComponents)
	{
		Component* thisComponent =
			GetComponentByTypeId(changedPrefabComponent->GetComponentTypeID());

		// if the gameobject instance does not have this component, means it has
		// been removed in that instance and that is ok
		if (thisComponent)
		{
			Component* unchangedPrefabGameObjectComponent = aUnchangedPrefabGameObject->GetComponentByTypeId(changedPrefabComponent->GetComponentTypeID());

			if (unchangedPrefabGameObjectComponent)
			{
				// Sync the values
				Engine::GameObjectPrefabReflector refl2(*thisComponent, unchangedPrefabGameObjectComponent);
				changedPrefabComponent->Reflect(refl2);
			}
			else
			{
				// if the unchanged component did not have the component
				// means we added the component in the modification, so do nothing
				// because we have already cloned the component from the prefab gameobject
				// int test = 0;
			}
		}
		else
		{
			// Means it has been removed in this instance, do not do anything
		}
	}

	// Sync newly added components
	for (Component* c : aChangedPrefabGameObject->myComponents)
	{
		const int componentTypeId = c->GetComponentTypeID();

		// If the unchanged does not contain a component in the changed one
		// means we added new into changed
		const bool wasComponentAdded =
			aUnchangedPrefabGameObject->GetComponentByTypeId(componentTypeId) == nullptr;

		if (wasComponentAdded)
		{
			// unchanged compare mot current

			// If this gameobject already contains the component, it has already been "overidden"
			// therefore we don't overwrite it
			//const bool isOveridden = GetComponentByTypeId(componentTypeId) != nullptr;
			//
			//if (!isOveridden)
			//{
			//	AddExistingComponent(c->Clone());
			//}

			if (GetComponentByTypeId(componentTypeId) == nullptr)
			{
				AddExistingComponent(c->Clone());
			}
		}
	}

	// Sync newly removed components
	for (Component* c : aUnchangedPrefabGameObject->myComponents)
	{
		const int componentTypeId = c->GetComponentTypeID();

		// If the changed does not contain a component in the unchanged one
		// means we removed a component from changed
		const bool wasComponentRemoved =
			aChangedPrefabGameObject->GetComponentByTypeId(componentTypeId) == nullptr;

		if (wasComponentRemoved)
		{
			Component* thisComponent = GetComponentByTypeId(componentTypeId);

			// If this gameobject already is removed, we cannot remove it
			const bool isRemoved = thisComponent == nullptr;

			if (!isRemoved)
			{
				RemoveComponentNoFree(thisComponent);
			}
		}
	}
}

void GameObject::SyncWithPrefabFull(GameObject* aBeforeChangedPrefabGameObject, GameObject* aAfterChangePrefabGameObject)
{
	// Sync this gameobject
	SyncWithPrefab(aBeforeChangedPrefabGameObject, aAfterChangePrefabGameObject);

	C::VectorOnStack<GameObject*, 140> thisChildren;
	C::VectorOnStack<GameObject*, 140> beforeChangedChildren;
	C::VectorOnStack<GameObject*, 140> afterChangeChildren;

	// Sync the children
	for (auto gameObjectIt = Engine::GameObjectChildIterator(*this); gameObjectIt != nullptr; gameObjectIt++)
	{
		thisChildren.Add(gameObjectIt.DerefAsRaw());
	}

	for (auto unchangedPrefabGameObjectIt = Engine::GameObjectChildIterator(*aBeforeChangedPrefabGameObject); unchangedPrefabGameObjectIt != nullptr; unchangedPrefabGameObjectIt++)
	{
		beforeChangedChildren.Add(unchangedPrefabGameObjectIt.DerefAsRaw());
	}

	for (auto changedPrefabGameObjectIt = Engine::GameObjectChildIterator(*aAfterChangePrefabGameObject); changedPrefabGameObjectIt != nullptr; changedPrefabGameObjectIt++)
	{
		afterChangeChildren.Add(changedPrefabGameObjectIt.DerefAsRaw());
	}

	for (const auto& g : thisChildren)
	{
		// Find the matching the gameobjects in the prefab
		const int prefabGameObjectID = g->myPrefabGameObjectID;

		auto beforeChangeFindResult = std::find_if(beforeChangedChildren.begin(), beforeChangedChildren.end(),
			[prefabGameObjectID](GameObject* aG1)
			{
				return prefabGameObjectID == aG1->myPrefabGameObjectID;
			});

		// If the gameobject did not exist in any of the
		// prefab gameobjects to compare against, it has
		// been removed, which is handled below in a seperate pass
		if (beforeChangeFindResult == beforeChangedChildren.end())
		{
			continue;
		}

		auto afterChangeFindResult = std::find_if(afterChangeChildren.begin(), afterChangeChildren.end(),
			[prefabGameObjectID](GameObject* aG1)
			{
				return prefabGameObjectID == aG1->myPrefabGameObjectID;
			});

		if (afterChangeFindResult == afterChangeChildren.end())
		{
			continue;
		}

		// If both contains the gameobject, means its still there and can be synced
		g->SyncWithPrefab(
			*beforeChangeFindResult,
			*afterChangeFindResult);
	}

	// Sync newly added gameobjects
	for (GameObject* g : afterChangeChildren)
	{
		const int prefabGameObjectID = g->myPrefabGameObjectID;

		const auto gameObjectFindResult =
			std::find_if(
				beforeChangedChildren.begin(),
				beforeChangedChildren.end(),
				[prefabGameObjectID](GameObject* aG)
				{
					return aG->myPrefabGameObjectID == prefabGameObjectID;
				});

		// If the before changed does not contain a component 
		// in the changed one means we added new into changed
		if (gameObjectFindResult == beforeChangedChildren.end())
		{
			// add the newly added gameobject

			// We do not recursively clone the gameobject
			// we do it manually ourself because its easier
			// for this case
			const bool recursively = false;

			// Clone the whole new gameobject hierarchy
			// meaning its children as well
			auto newGameObject = g->Clone(myScene->myGameObjectManager, true, recursively);

			assert(myPrefab && myPrefab->IsValid());

			// Add it to the internal list because otherwise, if we add a new gameobject
			// and then add children under that gameobject, it will search for the parent
			// in thisChildren
			thisChildren.Add(newGameObject.get());

			// Set the correct prefab gameobject
			newGameObject->myPrefabGameObject = myPrefab->Get().GetGameObject()->FindChildByPrefabGameObjectIDIncludingMe(newGameObject->GetPrefabGameObjectID());

			assert(newGameObject->myPrefabGameObject);

			// Find and set parent
			auto parentFindResult = std::find_if(
				thisChildren.begin(),
				thisChildren.end(),
				[g](GameObject* aG)
				{
					return g->GetTransform().GetParent()->GetGameObject()->GetPrefabGameObjectID() == aG->GetPrefabGameObjectID();
				});

			if (parentFindResult != thisChildren.end())
			{
				auto dick = *parentFindResult;

				newGameObject->GetTransform().SetParent2(&dick->myTransform);
			}
			else
			{
				// If no child was found as a parent, means the actual main prefab gameobject is the parent
				newGameObject->GetTransform().SetParent2(&myTransform);
			}
		}
	}

	// Sync newly removed gameobjects
	for (GameObject* g : beforeChangedChildren)
	{
		const int prefabGameObjectID = g->myPrefabGameObjectID;

		const auto gameObjectFindResult =
			std::find_if(
				afterChangeChildren.begin(),
				afterChangeChildren.end(),
				[prefabGameObjectID](GameObject* aG)
				{
					return aG->myPrefabGameObjectID == prefabGameObjectID;
				});

		// If the changed does not contain a gameobject in the unchanged one means we removed a gameobject from changed
		if (gameObjectFindResult == afterChangeChildren.end())
		{
			// Remove the gameobject if it exists
			const auto findResult =
				std::find_if(
					thisChildren.begin(),
					thisChildren.end(),
					[prefabGameObjectID](GameObject* aG)
					{
						return aG->myPrefabGameObjectID == prefabGameObjectID;
					});

			if (findResult != thisChildren.end())
			{
				std::vector<std::pair<Weak<GameObject>, Transform*>> childrenToRemove;

				// Remove the gameobject and all its children as well
				for (auto it = Engine::GameObjectChildIterator(*this); it != nullptr; it++)
				{
					if (it.DerefAsRaw() == *findResult)
					{
						// Remove the children first
						for (auto childItOfGameObjectToRemove = Engine::GameObjectChildIterator(*it.DerefAsRaw()); childItOfGameObjectToRemove != nullptr; childItOfGameObjectToRemove++)
						{
							GameObject* childGo = childItOfGameObjectToRemove.DerefAsRaw();

							for (auto& c2 : childGo->myComponents)
							{
								myScene->myGameObjectManager.myComponentSystem.RemoveComponent(c2);
							}

							if (childGo->GetTransform().GetParent() != nullptr)
							{
								childrenToRemove.push_back(
									std::make_pair(childGo->GetTransform().GetParent()->GetGameObjectShared(), &childGo->GetTransform()));
							}
							else
							{
								LOG_WARNING(LogType::Resource) << "Prefab tried to remove a child that has no parent";
							}

							myScene->myGameObjectManager.RemoveGameObject(childGo);
							thisChildren.RemoveCyclic(childGo);
						}

						GameObject* topGo = it.DerefAsRaw();

						// Remove the gameobject now
						for (auto& c : topGo->myComponents)
						{
							myScene->myGameObjectManager.myComponentSystem.RemoveComponent(c);
						}

						childrenToRemove.push_back(
							std::make_pair(topGo->GetTransform().GetParent()->GetGameObjectShared(), &topGo->GetTransform()));

						myScene->myGameObjectManager.RemoveGameObject(topGo);

						thisChildren.RemoveCyclic(topGo);

						break;
					}
				}

				for (auto& childPair : childrenToRemove)
				{
					auto child = childPair.second;

					if (auto parent = childPair.first.lock())
					{
						auto r = std::find_if(
							parent->myTransform.myChildren.begin(),
							parent->myTransform.myChildren.end(),
							[child](Transform* t)
							{
								return t == child;
							});

						if (r != parent->myTransform.myChildren.end())
						{
							parent->myTransform.myChildren.erase(r);
						}
					}
				}
			}
		}
	}

	struct ChangeParentStruct
	{
		Transform* myChild = nullptr;
		Transform* myNewParent = nullptr;
	};

	std::vector<ChangeParentStruct> parentChanges;

	// Sync the child gameobject hierarchy order and parent/child relationships
	{
		for (auto unchangedPrefabGameObjectIt = Engine::GameObjectChildIterator(*aBeforeChangedPrefabGameObject);
			unchangedPrefabGameObjectIt != nullptr;
			unchangedPrefabGameObjectIt++)
		{
			GameObject* unchanged = unchangedPrefabGameObjectIt.DerefAsRaw();

			// We find the respective child and verify its parent are the same
			if (GameObject* changed = aAfterChangePrefabGameObject->FindChildByPrefabGameObjectIDIncludingMe(unchanged->GetPrefabGameObjectID()))
			{
				// Check parent
				if (unchanged->GetTransform().GetParent()->GetGameObject()->GetPrefabGameObjectID() != changed->GetTransform().GetParent()->GetGameObject()->GetPrefabGameObjectID())
				{
					// Different parent
					const int parentPrefabGameObjectID = changed->GetTransform().GetParent()->GetGameObject()->GetPrefabGameObjectID();

					auto parentFindResult = std::find_if(
						thisChildren.begin(),
						thisChildren.end(),
						[parentPrefabGameObjectID](GameObject* aG)
						{
							return parentPrefabGameObjectID == aG->GetPrefabGameObjectID();
						});

					GameObject* target = FindChildByPrefabGameObjectIDIncludingMe(unchanged->GetPrefabGameObjectID());

					if (target)
					{
						if (parentFindResult != thisChildren.end())
						{
							auto g = *parentFindResult;

							ChangeParentStruct cps;
							cps.myChild = &target->GetTransform();
							cps.myNewParent = &g->myTransform;

							parentChanges.push_back(cps);
						}
						else
						{
							ChangeParentStruct cps;
							cps.myChild = &target->GetTransform();
							cps.myNewParent = &myTransform;

							parentChanges.push_back(cps);
						}
					}
				}
			}
		}
	}

	// Must be done outside the loop above
	for (const auto& cps : parentChanges)
	{
		if (cps.myChild == cps.myNewParent)
		{
			LOG_ERROR(LogType::Editor) << "Loading prefab changes tried setting itself to its parent, inform filip";
			continue;
		}

		cps.myChild->SetParent2(cps.myNewParent);
	}


	// TOODO: omg kan inte lira på 1:1, sluta iterera såhär, använd FindChildByPrefabGameObjectIDIncludingMe() istället 
	// 
	// Sync the child gameobject hierarchy order
	//{
	//	auto gameObjectIt = Engine::GameObjectChildIterator(*this);
	//	auto unchangedPrefabGameObjectIt = Engine::GameObjectChildIterator(*aBeforeChangedPrefabGameObject);
	//	auto changedPrefabGameObjectIt = Engine::GameObjectChildIterator(*aAfterChangePrefabGameObject);

	//	for (;
	//		gameObjectIt != nullptr &&
	//		unchangedPrefabGameObjectIt != nullptr &&
	//		changedPrefabGameObjectIt != nullptr;
	//		gameObjectIt++,
	//		unchangedPrefabGameObjectIt++,
	//		changedPrefabGameObjectIt++)
	//	{
	//		GameObject* unchanged = unchangedPrefabGameObjectIt.DerefAsRaw();
	//		GameObject* changed = changedPrefabGameObjectIt.DerefAsRaw();
	//		GameObject* target = gameObjectIt.DerefAsRaw();

	//		// If order is different within the children
	//		// this is done after fixing the parent/child relationships
	//		// meaning only the children can be in wrong order
	//		if (unchanged->GetPrefabGameObjectID() != changed->GetPrefabGameObjectID())
	//		{
	//			// det vår nått problem när man flyttar runt parents och sånt
	//			// och sen startar om, då blir prefab instansen fuckt med ett vitt gameobject i sig själv?
	//		}
	//	}
	//}
}

int GameObject::GetPrefabGameObjectID() const
{
	return myPrefabGameObjectID;
}

GameObject* GameObject::FindChildByPrefabGameObjectID(const int aPrefabGameObjectID)
{
	for (auto it = Engine::GameObjectChildIterator(*this); it != nullptr; it++)
	{
		if (it.DerefAsRaw()->GetPrefabGameObjectID() == aPrefabGameObjectID)
		{
			return it.DerefAsRaw();
		}
	}

	return nullptr;
}

GameObject* GameObject::FindChildByPrefabGameObjectIDIncludingMe(const int aPrefabGameObjectID)
{
	if (myPrefabGameObjectID == aPrefabGameObjectID)
	{
		return this;
	}

	return FindChildByPrefabGameObjectID(aPrefabGameObjectID);
}

void GameObject::ReflectLayerImgui(Engine::Reflector& aReflector, const std::string& aName, LayerMask& aLayerMask)
{
	aReflector.ReflectLambda([this, &aName, &aLayerMask]()
		{
			ImGui::PushID(&aLayerMask);

			ImGuiHelper::AlignedWidget2(aName.c_str(), Engine::Editor::ourAlignPercent);
			if (ImGui::Button(("Edit " + aName).c_str()))
			{
				ImGui::OpenPopup(aName.c_str());
			}

			if (ImGui::BeginPopup(aName.c_str()))
			{
				if (ImGui::BeginChild((aName + "layermasks").c_str(), ImVec2(300.f, 300.f), true, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar))
				{
					for (LayerMask mask = 1;
						mask <= static_cast<unsigned int>(eLayer::ALL);
						mask = mask << 1)
					{
						ImGui::PushID(mask);
						bool v = aLayerMask & mask;

						const std::string layerString = eLayer::ToString(mask);

						if (!layerString.empty())
						{
							if (ImGui::Checkbox(layerString.c_str(), &v))
							{
								if (v)
								{
									aLayerMask |= mask;
								}
								else
								{
									aLayerMask &= ~mask;
								}
							}
						}
						ImGui::PopID();
					}
				}
				ImGui::EndChild();
				ImGui::EndPopup();
			}

			ImGui::PopID();
		});
}

void GameObject::SetScene(Engine::Scene* aScene)
{
	myScene = aScene;
}

void GameObject::Update(float aDeltaTime)
{
}

void GameObject::Render()
{
}

bool GameObject::OnDestroy()
{
	return false;
}

void GameObject::Destroy()
{
	if (OnDestroy())
	{
		return;
	}

	myIsDestroyed = true;

	// Destroy its children as well
	for (auto& child : myTransform.GetChildren())
	{
		child->GetGameObject()->Destroy();
	}
}

void GameObject::SetActive(const bool aIsActive)
{
	if (aIsActive)
	{
		if (!myHasStarted && !myHasAwaked)
		{
			InternalAwake();
			InternalStart();
		}
	}

	myIsActive = aIsActive;

	for (auto& child : myTransform.GetChildren())
	{
		child->GetGameObject()->SetActive(aIsActive);
	}

	for (auto& comp : myTransform.GetGameObject()->myComponents)
	{
		if (comp)
		{
			aIsActive ? comp->OnEnable() : comp->OnDisable();
		}
	}

	for (auto& child : myTransform.GetChildren())
	{
		auto go = child->GetGameObject();
		for (auto& comp : go->myComponents)
		{
			if (comp)
			{
				aIsActive ? comp->OnEnable() : comp->OnDisable();
			}
		}
	}
}

void GameObject::SetName(const std::string& aName)
{
	myName = aName;
}

void GameObject::SetTag(eTag aTag)
{
	myTag = aTag;
}

bool GameObject::IsActive() const
{
	return myIsActive;
}

Transform& GameObject::GetTransform()
{
	return myTransform;
}

void GameObject::SetTransform(const Transform& aTransform)
{
	myTransform.SetPositionLocal(aTransform.GetPositionLocal());
	myTransform.SetRotationLocal(aTransform.GetRotationLocal());
	myTransform.SetScaleLocal(aTransform.GetScaleLocal());
}
void GameObject::SetTransform(const Mat4f& aMatrix4)
{
	Vec3f pos;
	Vec3f rot;
	Vec3f scale;
	Quatf rotation;

	aMatrix4.Decompose(pos, rotation, scale);

	Transform transform;
	transform.SetPosition(pos);
	transform.SetRotation(rotation);
	transform.SetScale(scale);

	SetTransform(transform);
}

const Transform& GameObject::GetTransform() const
{
	return myTransform;
}

void GameObject::AddExistingComponent(Component* aComponent)
{
	aComponent->ChangeParentGameObject(*this);

	myComponents.push_back(aComponent);

	// If it does not have a scene, means it probably a prefab gameobject
	if (myScene)
	{
		myScene->GetComponentSystem().AddExistingComponent(aComponent);
	}
}

void GameObject::RemoveComponentNoFree(Component* aComponent)
{
	for (int i = 0; i < myComponents.size(); i++)
	{
		if (aComponent == myComponents[i])
		{
			myComponents.erase(myComponents.begin() + i);
		}
	}

	// If no scene, means its probably a prefab gameobject
	if (myScene)
	{
		myScene->GetComponentSystem().RemoveComponent(aComponent);
	}
}

void GameObject::RemoveComponent(Component* aComponent)
{
	myScene->GetComponentSystem().ReleaseComponent(aComponent);

	for (int i = 0; i < myComponents.size(); i++)
	{
		if (aComponent == myComponents[i])
		{
			myComponents.erase(myComponents.begin() + i);
		}
	}
}

void GameObject::RemoveComponentInternal(Component* aComponent)
{
	for (int i = 0; i < myComponents.size(); i++)
	{
		if (aComponent == myComponents[i])
		{
			myComponents.erase(myComponents.begin() + i);
		}
	}
}

const std::string& GameObject::GetName() const
{
	return myName;
}

eTag GameObject::GetTag() const
{
	return myTag;
}

void GameObject::SetLayers(LayerMask aLayer, LayerMask aPhysicalLayerMask, LayerMask aDetectionLayerMask)
{
	if (aLayer == 0)
		int a = 0;

	myLayer = aLayer;
	myPhysicalLayerMask = aPhysicalLayerMask;
	myDetectionLayerMask = aDetectionLayerMask;
}

void GameObject::SetLayer(Layer aLayer)
{
	myLayer = aLayer;
}

void GameObject::SetPhysicalLayer(LayerMask aPhysicalLayerMask)
{
	myPhysicalLayerMask = aPhysicalLayerMask;
}

void GameObject::SetDetectionLayer(LayerMask aDetectionLayerMask)
{
	myDetectionLayerMask = aDetectionLayerMask;
}

LayerMask GameObject::GetLayer() const
{
	return myLayer;
}

LayerMask GameObject::GetPhysicalLayerMask() const
{
	return myPhysicalLayerMask;
}

LayerMask GameObject::GetDetectionLayerMask() const
{
	return myDetectionLayerMask;
}

bool GameObject::IsDestroyed() const
{
	return myIsDestroyed;
}
