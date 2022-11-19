#pragma once

#include "Engine/Renderer/Texture/FullscreenTexture.h"

namespace Engine
{
	class BlendTree2D;
	class BlendNode2D;
	class AnimationNodeEditorWindow;

	class BlendTree2DEditor
	{
	public:
		BlendTree2DEditor(AnimationNodeEditorWindow& myAnimEditor);

		void Draw(BlendTree2D& aBlendTree);

		void Reset();

	private:
		void DrawNodeCircles(BlendTree2D& aBlendTree);
		void DrawNodeCircle(const int aIndex, BlendNode2D& aNode, BlendTree2D& aBlendTree);

		void InvisibleSelectable(const void* aId, const ImVec2& aPosition, const ImVec2& aSize);

		// Converts position from -1 to 1 to window space
		// for position only
		Vec2f NormalizedSpaceToScreenSpace(const Vec2f aPosition) const;
		Vec2f ScreenSpaceToNormalizedSpace(const Vec2f aPosition) const;

		ImVec2 Vec2ToImVec2(const Vec2f& aVec) { return ImVec2(aVec.x, aVec.y); }
		Vec2f ImVec2ToVec2(const ImVec2& aVec) { return Vec2f(aVec.x, aVec.y); }

		void UpdateInputs(BlendTree2D& aBlendTree);
		void DrawNodeProperties(BlendTree2D& aBlendTree, BlendNode2D& aNode);
		void DrawGrid();
		void DrawNodeTriangulation(BlendTree2D& aBlendTree);
		void DrawLiveInstanceBlendValue(BlendTree2D& aBlendTree);

	private:
		AnimationNodeEditorWindow& myAnimEditor;

		Vec2f myWindowSize = Vec2f(350.f, 350.f);
		Vec2f myMinPadding = Vec2f(15.f, 10.f);

		Vec2f myChildMin;
		Vec2f myChildMax;

		int mySelectedNodeIndex = -1;

		float myGridIncrement = 0.25f;

		bool mySnapping = true;

		// FullscreenTexture myTexture;
	};
}