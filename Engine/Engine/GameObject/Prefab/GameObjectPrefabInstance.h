#pragma once
//
//#include "Engine\GameObject\GameObject.h"
//#include "Engine\ResourceManagement\ResourceRef.h"
//
//namespace Engine
//{
//	class GameObjectPrefabInstance : public GameObject
//	{
//	public:
//		const GameObjectPrefabRef& GetPrefab() const;
//
//		void AddExistingComponent(Component* aComponent) override;
//		void RemoveComponentNoFree(Component* aComponent) override;
//
//		// master prefab means if its the actual prefab gameobject that all
//		// instances depend on
//		bool IsMasterPrefab() const;
//
//	private:
//		friend class GameObjectPrefab;
//
//		// GameObjectPrefabRef myPrefab;
//	};
//}