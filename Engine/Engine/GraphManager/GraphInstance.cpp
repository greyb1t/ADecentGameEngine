#include "pch.h"
#include "GraphInstance.h"

#include "GraphNodeRegistry.h"
#include "Nodes/Base/GraphVariable.h"

#include "rapidjson/document.h"
#include <Engine/GameObject/GameObject.h>

using namespace rapidjson;

GraphInstance::GraphInstance(GameObject* aGameObject) : myGameObject(aGameObject)
{
}

GraphInstance::~GraphInstance()
{
}

void GraphInstance::LoadTreeFromFile()
{

}

void GraphInstance::ExecuteStart()
{
	for (auto& gni : myNodeInstancesInGraph)
	{
		if (gni->myNodeType->IsStartNode() && gni->GetNodeName() == "Start")
		{
			gni->Enter();
		}
	}
}
void GraphInstance::ExecuteNode(const std::string& aNodeName, const std::string& aEvent, const int aUUID)
{
	for (auto& gni : myNodeInstancesInGraph)
	{
		if (gni->myNodeType->IsStartNode())
		{
			if (gni->GetNodeName() == aNodeName)
			{
				Payload pL;

				pL.AddData<DataType::String, std::string>(aEvent);
				pL.AddData<DataType::Int, int>(aUUID);

				gni->EnterStartNode(pL);
			}
		}
	}
}
void GraphInstance::ExecuteNode(const std::string& aNodeName, const std::string& aEvent)
{
	for (auto& gni : myNodeInstancesInGraph) {
		if (gni->myNodeType->IsStartNode())
		{
			if (gni->GetNodeName() == aNodeName)
			{
				Payload pL;
				pL.AddData<DataType::String, std::string>(aEvent);
				gni->EnterStartNode(pL);
			}
		}
	}
}

void GraphInstance::ExecuteNode(const std::string& aNodeName)
{
	for (auto& gni : myNodeInstancesInGraph)
	{
		if (gni->GetNodeName() == aNodeName)
		{
			gni->Enter();
		}
	}
}
