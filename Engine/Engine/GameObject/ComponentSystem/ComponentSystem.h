#pragma once

#include "ComponentHandler.hpp"
#include "ComponentFactory.h"

namespace Engine
{
	class ComponentFactory;

	class ComponentSystem
	{
	public:
		ComponentSystem();
		~ComponentSystem();

		void Init();

		void Execute(eEngineOrder aOrder);
		void EditorExecute();
		void Render() const;

		template <typename T>
		void CreateHandlerIfNotExists(const int aId)
		{
			if (aId >= myComponentHandlers.size())
			{
				myComponentHandlers.resize(aId + 1);
			}

			if (myComponentHandlers[aId].GetID() == -1)
			{
				myComponentHandlers[aId].SetID(aId);

				// for debugging only
				myComponentHandlers[aId].SetClassID(typeid(T).name());
			}
		}

		void CreateHandlerIfNotExists2(const std::string& aClassName, const int aId)
		{
			if (aId >= myComponentHandlers.size())
			{
				myComponentHandlers.resize(aId + 1);
			}

			if (myComponentHandlers[aId].GetID() == -1)
			{
				myComponentHandlers[aId].SetID(aId);

				// for debugging only
				myComponentHandlers[aId].SetClassID(aClassName);
			}
		}

		template<class T, class ... Args>
		T* AddComponent(GameObject* aGameObject, Args&&... args)
		{
			const int id = ComponentFactory::GetInstance().GetComponentId<T>();

			CreateHandlerIfNotExists<T>(id);

			T* component = DBG_NEW T(aGameObject, std::forward<Args&&>(args)...);

			component->OnConstruct();

			myComponentHandlers[id].AddComponent(component);

			return component;
		}

		void ReleaseComponent(Component* aComponent);

		// Does not delete it from memory
		void RemoveComponent(Component* aComponent);

		// Only used to add components that were already created from a copy
		void AddExistingComponent(Component* aComponent);

		template<class T>
		ComponentHandler& GetComponentHandler();

	private:
		friend class GameObjectManager;
		friend class GameObject;

		std::vector<ComponentHandler> myComponentHandlers;
	};

	template<class T>
	inline ComponentHandler& ComponentSystem::GetComponentHandler()
	{
		auto index = ComponentFactory::GetInstance().GetComponentId<T>();
		return myComponentHandlers[index];
	}
}
