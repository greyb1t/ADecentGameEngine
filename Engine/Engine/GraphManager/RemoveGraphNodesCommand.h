#pragma once

#include "Command.h"

class GraphNodeInstance;
class GraphInstance;

class RemoveGraphNodesCommand : public Command
{
public:
	RemoveGraphNodesCommand(
		GraphInstance* aGraphInstance,
		const std::vector<GraphNodeInstance*>& aGraphNodeInstances);

	bool Execute() override;
	bool Undo() override;

private:
	GraphInstance* myGraphInstance = nullptr;

	std::vector<GraphNodeInstance*> myGraphNodeInstances;
};