#pragma once

#include "EditorWindow.h"
#include "Engine\Renderer\RendererSceneDebugOutput.h"
#include "Engine\ResourceManagement\ResourceRef.h"

namespace Engine
{
	class ResourceReferences;

	class SceneWindow : public EditorWindow
	{
	public:
		SceneWindow(Editor& aEditor, ResourceReferences& aResourceReferences);

		void Draw(const float aDeltaTime) override;

		std::string GetName() const override { return "Active Scene"; }

		const Vec2f& GetWindowPosition() const;
		const Vec2f& GetWindowSize() const;

		const Vec2f& GetWindowItemRectMin() const;
		const Vec2f& GetWindowItemRectSize() const;

	private:
		void DrawPlayStopButton();
		void DrawPauseResumeButton();
		void DrawRecentScenes();
		void DrawWelcomeWidget();
		void DrawCameraSpeed();

		void DrawGridCheckbox();
		void DrawSceneRenderDebugOutputCombo();

	private:
		std::vector<Path> myRecentScenes;
		size_t mySelectedRecentSceneIndex = static_cast<size_t>(-1);
		std::string myWelcomeMessage;

		Vec2f myWindowItemRectMin;
		Vec2f myWindowItemRectSize;

		Vec2f myWindowPosition;
		Vec2f myWindowSize;

		std::vector<TextureRef> myEditorPics;

		TextureRef myEditorTexture;
	};
}
