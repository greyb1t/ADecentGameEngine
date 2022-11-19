#include "pch.h"
#include "RemoveGraphNodesCommand.h"

#include "GraphInstance.h"
#include "GraphManager.h"
#include "Nodes/Base/GraphNodeInstance.h"

RemoveGraphNodesCommand::RemoveGraphNodesCommand(
	GraphInstance* aGraphInstance,
	const std::vector<GraphNodeInstance*>& aGraphNodeInstances)
	: myGraphInstance(aGraphInstance),
	myGraphNodeInstances(aGraphNodeInstances)
{
}

bool RemoveGraphNodesCommand::Execute()
{
	for (auto& graphNodeInstance : myGraphNodeInstances)
	{
		auto it = myGraphInstance->myNodeInstancesInGraph.begin();
		while (it != myGraphInstance->myNodeInstancesInGraph.end())
		{
			if ((*it)->myUID.ToUInt() == graphNodeInstance->myUID.ToUInt())
			{
				it = myGraphInstance->myNodeInstancesInGraph.erase(it);
				break;
			}
			else
			{
				++it;
			}
		}
	}

	return true;
}

bool RemoveGraphNodesCommand::Undo()
{
	for (auto& graphNodeInstance : myGraphNodeInstances)
	{
		myGraphInstance->myNodeInstancesInGraph.push_back(graphNodeInstance);
	}

	return true;
}
