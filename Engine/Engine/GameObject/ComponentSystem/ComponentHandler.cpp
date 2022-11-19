#include "pch.h"
#include "ComponentHandler.hpp"

#include "Engine/GameObject/GameObject.h"

Engine::ComponentHandler::ComponentHandler()
{
}

Engine::ComponentHandler::~ComponentHandler()
{
}

void Engine::ComponentHandler::Execute(eEngineOrder aOrder)
{
	// Check for destroyed components
	for (auto it = myActiveComponents.begin(); it != myActiveComponents.end();)
	{
		Component* c = *it;
		if (c != nullptr && c->IsDestroyed())
		{
			c->GetGameObject()->RemoveComponentInternal(c);
			delete c;
			c = nullptr;
		}
		if (c == nullptr || c->IsDestroyed())
		{
			it = myActiveComponents.erase(it);
		}
		else
		{
			++it;
		}
	}

	for (size_t i = 0; i < myActiveComponents.size(); ++i)
	{
		Component* c = myActiveComponents[i];

		if (c && c->IsActive() && c->myGameObject && c->myGameObject->IsActive() && c->HasStarted() && !c->IsDestroyed())
		{
			assert(!c->myIsDestroyed && "Why is a destroyed calling execute?");
			assert(c->myHasStarted && "must have started first");

			c->Execute(aOrder);
		}
	}

	/*
		En global function

		thread_local StackAllocator* myAllocator;

		StackAlloctor.Allocate<Struct>();

		T* Allocate
		{
			std::thread::get_id();
		}
	*/

	GetEngine().GetParallelizationThreadPool().ParallellLoop(
		myActiveComponents.size(),
		[aOrder, this](const int aStart, const int aEnd)
		{
			for (int i = aStart; i < aEnd; ++i)
			{
				auto& c = myActiveComponents[i];

				if (c && c->IsActive() &&
					c->myGameObject && c->myGameObject->IsActive() &&
					c->HasStarted() &&
					!c->IsDestroyed())
				{
					assert(!c->myIsDestroyed && "Why is a destroyed calling execute?");
					assert(c->myHasStarted && "must have started first");

					c->ExecuteParallelized(aOrder);
				}
			}
		});

	/*
	for (Component* c : myActiveComponents)
	{
		GetEngine().GetParallelizationThreadPool().PushTask([aOrder, c]()
			{
				if (c->IsActive() && c->myGameObject->IsActive() && c->HasStarted())
				{
					assert(!c->myIsDestroyed && "Why is a destroyed calling execute?");
					assert(c->myHasStarted && "must have started first");

					c->ExecuteParallelized(aOrder);
				}
			});
	}

	GetEngine().GetParallelizationThreadPool().WaitForTasks();
	*/

	//std::for_each(
	//	std::execution::par_unseq,
	//	myActiveComponents.begin(),
	//	myActiveComponents.end(),
	//	[aOrder](Component* c)
	//	{
	//		if (c->IsActive() && c->myGameObject->IsActive() && c->HasStarted())
	//		{
	//			assert(!c->myIsDestroyed && "Why is a destroyed calling execute?");
	//			assert(c->myHasStarted && "must have started first");

	//			c->ExecuteParallelized(aOrder);
	//		}
	//	});

	for (size_t i = 0; i < myActiveComponents.size(); ++i)
	{
		Component* c = myActiveComponents[i];

		if (c && c->IsActive() &&
			c->myGameObject && c->myGameObject->IsActive() &&
			c->HasStarted() &&
			!c->IsDestroyed())
		{
			assert(!c->myIsDestroyed && "Why is a destroyed calling execute?");
			assert(c->myHasStarted && "must have started first");

			c->PostExecute();
		}
	}
}

void Engine::ComponentHandler::EditorExecute()
{
	for (auto& c : myActiveComponents)
	{
		if (c->IsActive() &&
			c->myGameObject->IsActive() &&
			!c->IsDestroyed())
		{
			assert(!c->myIsDestroyed && "Why is a destroyed calling execute?");
			c->EditorExecute();
		}
	}
}

void Engine::ComponentHandler::Render() const
{
	GetEngine().GetParallelizationThreadPool().ParallellLoop(
		myActiveComponents.size(),
		[this](const int aStart, const int aEnd)
		{
			for (int i = aStart; i < aEnd; ++i)
			{
				auto& c = myActiveComponents[i];

				if (c && c->IsActive() &&
					c->myGameObject && c->myGameObject->IsActive() &&
					!c->IsDestroyed())
				{
					assert(!c->myIsDestroyed && "Why is a destroyed calling execute?");

					c->RenderParallelized();
				}
			}
		});

	for (auto& c : myActiveComponents)
	{
		if (c && c->IsActive() &&
			c->myGameObject && c->myGameObject->IsActive() &&
			!c->IsDestroyed())
		{
			assert(!c->myIsDestroyed && "Why is a destroyed calling render?");

			c->Render();
		}
	}
}

void Engine::ComponentHandler::AddComponent(Component* aComponent)
{
	aComponent->myID = myID;

	myActiveComponents.push_back(aComponent);
}

void Engine::ComponentHandler::RemoveComponent(Component* aComponent)
{
	auto it = std::find(myActiveComponents.begin(), myActiveComponents.end(), aComponent);

	// assert(it != myActiveComponents.end() && "Failed removing component didn't exist!");

	if (it != myActiveComponents.end())
	{
		myActiveComponents.erase(it);
	}

	//aComponent->myIsDestroyed = true;
}
