#pragma once

#include "Engine/EngineOrder.h"
#include "Engine/GameObject/Components/Component.h"

namespace Engine
{
	class GameObjectManager;

	class ComponentHandler
	{
	public:
		ComponentHandler();
		~ComponentHandler();

		void Execute(eEngineOrder aOrder);
		void EditorExecute();
		void Render() const;

		void AddComponent(Component* aComponent);

		template<class T>
		void EachActiveComponent(const std::function<void(T*)>& aCallback);

		void RemoveComponent(Component* aComponent);
		void SetMask(unsigned int aMask) { myExecutionMask = aMask; }
		unsigned int GetMask() const { return myExecutionMask; }

		bool HasID() const { return myID != -1; }
		void SetID(const int aID) { myID = aID; }
		int GetID() const { return myID; }

		void SetClassID(const std::string& aClassId) { myClassID = aClassId; }
		const std::string& GetClassID() const { return myClassID; }

	private:
		friend class GameObjectManager;

		std::vector<Component*>	myActiveComponents;

		unsigned int myExecutionMask = UPDATE;
		int myID = -1;

		// only used for debugging
		std::string myClassID = "";
	};

	template<class T>
	inline void ComponentHandler::EachActiveComponent(const std::function<void(T*)>& aCallback)
	{
		for (auto& c : myActiveComponents)
		{
			if (c->IsActive() && c->myGameObject->IsActive())
			{
				aCallback(reinterpret_cast<T*>(c));
			}
		}
	}
}
