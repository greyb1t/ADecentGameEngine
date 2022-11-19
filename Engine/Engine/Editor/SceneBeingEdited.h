#pragma once

#include "Selection/Selection.h"
#include "Engine\Utils\Flags.h"

namespace Engine
{
	class FolderScene;
	class Editor;

	enum GizmosFlags
	{
		GizmosFlags_Translation = 1 << 0,
		GizmosFlags_Rotation = 1 << 1,
		GizmosFlags_Scaling = 1 << 2,
	};
	CREATE_FLAG_OF_ENUM(GizmosFlags);

	// Wrapping the real scene with the purpose of keeping a unique
	// editor state for each scene, e.g. each scene have different selected gameobjects
	class SceneBeingEdited
	{
	public:
		SceneBeingEdited(Editor& aEditor);
		~SceneBeingEdited();

		void InitFromJson(const nlohmann::json& aJson);
		nlohmann::json ToJson() const;

		// Clears all the selection state and stuff like that
		void Reset();

		FolderScene* GetScene();
		const FolderScene* GetScene() const;
		void SetScene(Shared<FolderScene> aScene);

		Selection& GetSelection();

		void Update();
		void EndFrame();

		void UpdateGizmos(
			const float aWindowX,
			const float aWindowY,
			const float aWindowWidth,
			const float aWindowHeight);
		void DrawGizmos(
			const GizmosFlags aFlags,
			const float aWindowX,
			const float aWindowY,
			const float aWindowWidth,
			const float aWindowHeight, 
			Mat4f& aMatrixOut) const;

		void UpdateGizmosKeyBinds();

		void SetGridY(const float aY);
		void SetSnapping(const float aSnapping);
		float GetSnapping() const;

	private:
		void DrawGrid();

	private:
		friend class SceneWindow;

		Editor& myEditor;

		Shared<FolderScene> myScene;

		Selection mySelection;

		int myGizmoType = 7;
		float mySnap[3] = { 0.0f, 0.0f, 0.0f };
		std::vector<Shared<GameObject>> myTransformedGameObjects;
		std::vector<Mat4f> myStartTransforms;
		std::vector<Mat4f> myEndTransforms;
		std::vector<Mat4f> myStartTransformsInv;
		bool myStartedUsingGizmos = false;

		Mat4f myGizmosTransform;
		Mat4f myStartGizmosTransform;

		Mat4f myStartTransform;
		Mat4f myStartTransformInverse;

		bool myDrawGrid = true;
		float myGridY = 0.f;

		mutable bool myStopDrawingGizmos = false;
	};
}