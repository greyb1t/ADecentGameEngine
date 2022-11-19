#pragma once
#include "Engine/GraphManager/Nodes/Base/GraphNodeBase.h" 

namespace Engine
{
	class FolderScene;
}

class GraphNode_LoadScene : public GraphNodeBase
{
public:
	GraphNode_LoadScene();

	int OnExec(class GraphNodeInstance* aNodeInstance) override;
	bool WaitForScene(const std::string& aSceneName);

	bool LoadScene(const std::string aSceneName, const std::string aFolderName, Shared<Engine::FolderScene> aEmptyScenePtr);

	virtual std::string GetNodeName() const override { return "Load Scene"; }
	virtual std::string GetNodeTypeCategory() const override { return "Scene"; }
};

