#include "pch.h"
#include "ScriptsComponent.h"
#include "Engine/Engine.h"
#include "Engine/Reflection/Reflector.h"
#include <Engine/ResourceManagement/Resources/VisualScriptResource.h>
#include <Engine/GraphManager/VisualScript.h>
#include "Engine/GraphManager/VisualGraphPair.h"

Engine::ScriptsComponent::ScriptsComponent(GameObject* aGameObject)
	: Component(aGameObject)
{

}

GraphInstance* Engine::ScriptsComponent::GetGraphInstance()
{
	return myGraphInstance;
}

void Engine::ScriptsComponent::Start()
{
	if (myGraph && myGraph->IsValid())
	{
		myGraphInstance = myGraph->Get().CreateInstance(myGameObject);

		if (myGraphInstance)
		{
			myGraphInstance->ExecuteStart();
		}
		else
		{
			LOG_ERROR(LogType::Engine) << "Cannot create visual script: " << myGraph->GetPath();
		}
	}
}

void Engine::ScriptsComponent::Execute(eEngineOrder aOrder)
{
	if (myGraphInstance != nullptr)
	{
		myGraphInstance->ExecuteNode("Update");
	}
}
void Engine::ScriptsComponent::Reflect(Reflector& aReflector)
{
	//Must be in all components 
	Component::Reflect(aReflector);
	VisualGraphPair pair(myGraph);

	if (myGraphInstance)
	{
		pair.myGraphInstance = myGraphInstance;
	}

	aReflector.Reflect(pair, "Script");
}