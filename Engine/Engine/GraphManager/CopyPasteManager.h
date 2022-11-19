#pragma once

#include "Nodes/Base/GraphNodeInstance.h"

class GraphManager;

class CopyPasteManager
{
public:
	CopyPasteManager(GraphManager& aGraphManager);

	bool Init();

	void CopySelectedNodes();
	void PasteNodes();

private:
	GraphManager& myGraphManager;

	std::vector<GraphNodeInstance> myCopiedNodes;
};