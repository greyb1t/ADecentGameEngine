#include "pch.h"
#include "ComponentSystem.h"
#include "../Components/Component.h"
#include "ComponentFactory.h"

Engine::ComponentSystem::ComponentSystem()
{
}

Engine::ComponentSystem::~ComponentSystem()
{
}

void Engine::ComponentSystem::Init()
{
	for (auto& r : ComponentFactory::GetInstance().GetRegisteredComponents())
	{
		const int id = r->myComponentTypeId;

		if (id >= myComponentHandlers.size())
		{
			myComponentHandlers.resize(id + 1);
		}

		const bool initialized = myComponentHandlers[id].GetID() != -1;

		if (!initialized)
		{
			myComponentHandlers[id].SetID(id);

			// for debugging only
			myComponentHandlers[id].SetClassID(r->myClassName);
		}

		myComponentHandlers[id].SetID(id);
		myComponentHandlers[id].SetMask(r->mask);
	}
}

void Engine::ComponentSystem::Execute(eEngineOrder aOrder)
{
	//ZoneScopedN("ComponentSystem::Execute");
	ZoneNamedN(zone1, "ComponentSystem::Execute", true);

	for (auto& it : myComponentHandlers)
	{
		ZoneNamedN(zone2, "Ite", true);
		ZoneTextV(zone2, it.GetClassID().c_str(), it.GetClassID().size());

		if (aOrder & it.GetMask())
		{
			it.Execute(aOrder);
		}
	}
}

void Engine::ComponentSystem::EditorExecute()
{
	ZoneNamedN(zone1, "ComponentSystem::EditorExecute", true);

	for (auto& it : myComponentHandlers)
	{
		ZoneNamedN(zone2, "Ite", true);
		ZoneTextV(zone2, it.GetClassID().c_str(), it.GetClassID().size());

		it.EditorExecute();
	}
}

void Engine::ComponentSystem::Render() const
{
	ZoneNamedN(zone1, "ComponentSystem::Render", true);

	for (auto& it : myComponentHandlers)
	{
		ZoneNamedN(zone2, "Ite", true);
		ZoneTextV(zone2, it.GetClassID().c_str(), it.GetClassID().size());

		it.Render();
	}
}

void Engine::ComponentSystem::ReleaseComponent(Component* aComponent)
{
	RemoveComponent(aComponent);

	// myComponentFactory.FreeComponent(aComponent);
	delete aComponent;
}

void Engine::ComponentSystem::RemoveComponent(Component* aComponent)
{
	const int id = aComponent->myID;

	if (id >= myComponentHandlers.size())
	{
		return;
		assert(myComponentHandlers.size() > id);
	}

	myComponentHandlers[id].RemoveComponent(aComponent);
}

void Engine::ComponentSystem::AddExistingComponent(Component* aComponent)
{
	assert(aComponent->myID != -1 && "what");

	const int id = aComponent->myID;

	// NOTE: This is not true for rigidbodies***
	// NOTE : should already exist since we copy the whole component system)
	CreateHandlerIfNotExists2(typeid(aComponent).name(), id);

	myComponentHandlers[id].AddComponent(aComponent);
}
