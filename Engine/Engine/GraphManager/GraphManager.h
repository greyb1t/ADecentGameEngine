#pragma once
#include <imgui_node_editor.h>

#include "Engine/ResourceManagement/ResourceRef.h"
#include "Nodes/Base/GraphVariable.h"
#include "GraphInstance.h"

namespace Engine
{
	class VisualScriptResource;
	class VisualScriptWindow;
}

class GraphInstance;

namespace ax { namespace NodeEditor { namespace Detail { struct EditorContext; } } }

class CopyPasteManager;
class UndoRedoManager;

namespace ed = ax::NodeEditor;
class GraphManager
{
public:
	GraphManager(
		Engine::VisualScriptWindow& aVisualGraphWindow,
		GraphInstance* aGraphInstance,
		const Path& aPath);

	~GraphManager();

	void Load();

	// void ReTriggerUpdateringTrees();

	void PreFrame(float aTimeDelta);
	void ConstructEditorTreeAndConnectLinks();
	void PostFram();

	// void ReTriggerTree(std::string aSpecificNodeName = std::string("Start"));
	void ExecuteTreeWithPayload(std::string aStartNodeName, Payload aPayload);
	void ExecuteTreeWithPayload(std::tuple<std::string, Payload> aTuple);
	void SaveTreeToFile();
	// void LoadTreeFromFile();
	static void ShowFlow(GraphInstance* aInstance, int aLinkID);

	ax::NodeEditor::Detail::EditorContext* GetContext();
	std::vector<class GraphNodeInstance*>& GetNodeInstancesInGraph();

	bool IsEditingInstance() const;
	bool IsEditingFile() const;

private:
	friend class CopyPasteManager;
	friend class AddGraphNodesCommand;
	friend class RemoveGraphNodesCommand;

	// Engine::ResourceRef<Engine::VisualGraphResource> myVisualGraphResource;
	TextureRef myBlueprintBackground;

	CopyPasteManager* myCopyPasteManager = nullptr;
	UndoRedoManager* myUndoRedoManager = nullptr;

	void WillBeCyclic(class GraphNodeInstance* aFirst, bool& aIsCyclic, class GraphNodeInstance* aBase);

	class GraphNodeInstance* GetNodeFromPinID(unsigned int aID);
	GraphNodeInstance* GetNodeFromNodeID(unsigned int aID);
	void DrawTypeSpecificPin(struct GraphNodePin& aPin, class GraphNodeInstance* aNodeInstance);

	bool myLikeToSave = false;
	bool myLikeToShowFlow = false;
	char* myMenuSeachField = nullptr;
	bool mySetSearchFokus = true;
	bool myShouldCreateVariable = false;
	char* myNewVarName = DBG_NEW char[255]();
	std::string myNewVarTypeLabel = "Select Type...";
	DataType myNewVarType = DataType::Unknown;
	int mySelectedVariableIndex = -1;
	std::string myVariableErrorMessage;

	Engine::VisualScriptWindow& myVisualGraphWindow;

	Path myPath;

	// NOTE(filip):
	// We have this static to because we need to access this in ShowFlow() to 
	// prevent other other visual scripts from showing flow when it not the one being edited
	// This assumes we only ever have one Visual Script Editor open at a time!
	static inline GraphInstance* myGraphInstance = nullptr;

	// bool myIsOpen = true;

	ed::EditorContext* myContext = nullptr;
};
