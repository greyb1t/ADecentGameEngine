#pragma once

#include "Engine\Animation\Playable.h"

namespace Engine
{
	class BlendNode2D
	{
	public:
		BlendNode2D() = default;

		BlendNode2D(const Vec2f aPosition, Owned<Playable> aPlayable);

		nlohmann::json ToJson() const;
		void InitFromJson(const nlohmann::json& aJson, AnimationMachine& aMachine);

		void SetPosition(const Vec2f& aPosition);
		const Vec2f& GetPosition() const;

		Playable* GetPlayable();
		const Playable* GetPlayable() const;

		float GetRotationRadians() const;

		void RecalculateRotation();

	private:
		friend class AnimationNodeEditorWindow;
		friend class BlendTree2DEditor;

		// Position between -1 (left) and 1 (right)
		// 0, 0 is center
		Vec2f myPosition;
		float myRotationRadians = 0.f;

		Owned<Playable> myPlayable;
	};
}