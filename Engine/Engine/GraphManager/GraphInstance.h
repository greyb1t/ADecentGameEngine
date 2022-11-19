#pragma once

#include <imgui_node_editor.h>
#include "Nodes/Base/GraphNodeInstance.h"

namespace ed = ax::NodeEditor;

class GameObject;

struct EditorLinkInfo
{
	ed::LinkId Id;
	ed::PinId  InputId;
	ed::PinId  OutputId;
};

class GraphInstance
{
public:
	GraphInstance(GameObject* aGameObject);
	~GraphInstance();

	void LoadTreeFromFile();

	void ExecuteStart();

	void ExecuteNode(const std::string& aNodeName);

	void ExecuteNode(const std::string& aNodeName, const std::string& aEvent, const int aUUID);

	void ExecuteNode(const std::string& aNodeName, const std::string& aEvent);

public:
	std::vector<GraphNodeInstance*> myNodeInstancesInGraph;
	std::vector<GraphVariable> myGraphVariables;

	// it is used in the GraphManager and used for saving, therefore this shit is here
	// even tho it is not need for the graph itself to execute properly
	ImVector<EditorLinkInfo> myLinks;

	// NOTE(filip): must be high number otherwise GraphManager breaks
	// when trying to delete a node and the actual link does not get deleted
	int myNextLinkIdCounter = 1000;

	GameObject* myGameObject = nullptr;

	int myMaxUID = -1;
	// std::vector<unsigned int> myAllUIDs;
};
