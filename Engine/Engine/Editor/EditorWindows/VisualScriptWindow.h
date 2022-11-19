#pragma once

#include "EditorWindow.h"
#include "Engine/GraphManager/GraphInstance.h"

class GraphInstance;
class GraphManager;

namespace Engine
{
	class VisualScriptWindow : public EditorWindow
	{
	public:
		VisualScriptWindow(Editor& aEditor);
		~VisualScriptWindow();

		bool InitExistingInstance(GraphInstance* aGraphInstance, const Path& aPath);
		bool InitNewInstance(Owned<GraphInstance> aInstance, const Path& aSavePath);

		void Draw(const float aDeltaTime) override;
		ImGuiWindowFlags GetWindowFlags() const override;

		std::string GetName() const override { return "Visual Script Editor"; }

		void OnEditorChangeMode(const EditorMode aMode) override;

	private:
		friend class GraphManager;

		Owned<GraphInstance> myInstance;

		Owned<GraphManager> myGraphManager;
	};
}
