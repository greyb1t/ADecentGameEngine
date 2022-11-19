#pragma once

class GameObject;
class Component;

namespace Engine
{
	struct SingletonComponentEntry
	{
		GameObject* myGameObject = nullptr;
		Component* myComponent = nullptr;
	};

	class SingletonComponentFactory
	{
	public:
		struct SingletonComponent
		{
			int myID = -1;
			std::string myGameObjectName;
			int myComponentID = -1;
		};

		static SingletonComponentFactory& GetInstance()
		{
			static SingletonComponentFactory factory;
			return factory;
		}

		template <typename T>
		void RegisterSingletonComponent(const std::string& aGameObjectName)
		{
			const int id = GetSingletonComponentId<T>();

			if (id >= ourSingletonComponents.size())
			{
				ourSingletonComponents.resize(id + 1);
			}

			SingletonComponent singletonComponent;
			singletonComponent.myID = id;
			singletonComponent.myGameObjectName = aGameObjectName;
			singletonComponent.myComponentID = ComponentFactory::GetInstance().GetComponentId<T>();

			ourSingletonComponents[id] = singletonComponent;
		}

		template <typename T>
		int GetSingletonComponentId()
		{
			static int id = ourSingletonComponentIdCounter++;
			return id;
		}

		std::vector<SingletonComponent>& GetSingletonComponents()
		{
			return ourSingletonComponents;
		}

	private:
		std::vector<SingletonComponent> ourSingletonComponents;

		int ourSingletonComponentIdCounter = 0;
	};
}