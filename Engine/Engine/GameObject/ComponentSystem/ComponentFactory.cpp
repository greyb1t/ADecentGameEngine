#include "pch.h"
#include "ComponentFactory.h"

#include "ComponentSystem.h"
#include "Engine/GameObject/Components/Component.h"

void Engine::ComponentFactory::RegisterComponentInternal(const RegisteredComponentStruct& aComponent)
{
	const int id = aComponent.myComponentTypeId;

	RegisteredComponentStruct* newRegComp = DBG_NEW RegisteredComponentStruct(aComponent);

	assert(std::find_if(
			   myRegisteredComponents.begin(),
			   myRegisteredComponents.end(),
			   [id](const RegisteredComponentStruct* aRegComp)
			   {
				   if (aRegComp == nullptr)
				   {
					   return false;
				   }

				   return aRegComp->myComponentTypeId == id;
			   })
			== myRegisteredComponents.end()
		&& "already exists");

	if (id >= (static_cast<int>(myRegisteredComponents.size()) - 1))
	{
		myRegisteredComponents.resize(id + 1);
	}

	std::hash<std::string> h;
	myComponentNameHashToRegisteredComponent[h(aComponent.myName)] = id;

	myRegisteredComponents[id] = newRegComp;
}

Engine::RegisteredComponentStruct* Engine::ComponentFactory::GetRegisteredComponentFromId(
	const int aComponentTypeId)
{
	// out of range
	if (aComponentTypeId >= myRegisteredComponents.size())
	{
		return nullptr;
	}

	return myRegisteredComponents[aComponentTypeId];
}

Engine::RegisteredComponentStruct*
Engine::ComponentFactory::GetRegisteredComponentFromComponentNameHash(const size_t aHash)
{
	auto hashFind = myComponentNameHashToRegisteredComponent.find(aHash);

	// out of range
	if (hashFind == myComponentNameHashToRegisteredComponent.end())
	{
		return nullptr;
	}

	return myRegisteredComponents[hashFind->second];
}

int Engine::ComponentFactory::GetRegisteredComponentsCount()
{
	return static_cast<int>(myRegisteredComponents.size());
}

const std::vector<Engine::RegisteredComponentStruct*>& Engine::ComponentFactory::GetRegisteredComponents()
{
	return myRegisteredComponents;
}
