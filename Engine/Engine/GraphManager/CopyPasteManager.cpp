#include "pch.h"
#include "CopyPasteManager.h"
#include "GraphManager.h"
#include "imgui_node_editor_internal.h"
#include "Nodes/Base/GraphNodeInstance.h"
#include "UndoRedoManager.h"
#include "AddGraphNodesCommand.h"

CopyPasteManager::CopyPasteManager(GraphManager& aGraphManager)
	: myGraphManager(aGraphManager)
{
}

bool CopyPasteManager::Init()
{
	return true;
}

void CopyPasteManager::CopySelectedNodes()
{
	myCopiedNodes.clear();

	auto ctx = myGraphManager.GetContext();

	const auto& objects = ctx->GetSelectedObjects();

	for (const auto& object : objects)
	{
		auto lol2 = object->ID();
		assert(lol2.IsNodeId());

		auto node = myGraphManager.GetNodeFromNodeID(lol2.Get());

		myCopiedNodes.push_back(*node);
	}
}

void CopyPasteManager::PasteNodes()
{
	auto ctx = myGraphManager.GetContext();

	std::vector<GraphNodeInstance*> graphNodesInstancesToAdd;

	for (const auto& node : myCopiedNodes)
	{
		GraphNodeInstance* newObj = DBG_NEW GraphNodeInstance(node);
		newObj->myUID.SetUID(UID(true));

		// We cannot rely on GraphManager because when creating a node manually,
		// it never actually sets myEditorPosition.
		// weird
		ImVec2 nodePos = ed::GetNodePosition(node.myUID.ToUInt());

		const float newPosOffset = 60.f;

		newObj->myHasSetEditorPos = false;
		newObj->myEditorPos[0] = nodePos.x + newPosOffset;
		newObj->myEditorPos[1] = nodePos.y + newPosOffset;

		//myGraphManager.myNodeInstancesInGraph.push_back(newObj);

		graphNodesInstancesToAdd.push_back(newObj);

		// auto newObj2 = ctx->FindObject();
		// ctx->SelectObject(newObj2);
	}

	// Add the all in one command
	// this way we can UNDO them all in one command as well
	auto cmd = DBG_NEW AddGraphNodesCommand(myGraphManager.myGraphInstance, graphNodesInstancesToAdd);
	myGraphManager.myUndoRedoManager->ExecuteCommand(cmd);

	// Deselect all prev objects
	for (auto& graphNodeInstance : myCopiedNodes)
	{
		const auto id = graphNodeInstance.myUID.ToUInt();
		ed::DeselectNode(id);
	}

	/*
	// Select new nodes
	for (auto& graphNodeInstance : graphNodesInstancesToAdd)
	{
		const auto id = graphNodeInstance->myUID.ToUInt();
		ed::SelectNode(id);
	}
	*/
}
