#pragma once

#include "EditorWindow.h"
#include "Engine/AnimationCurve/CurveEditor.h"
#include "Engine/ResourceManagement/ResourceRef.h"

namespace Engine
{
	class AnimationCurveEditorWindow : public EditorWindow
	{
	public:
		AnimationCurveEditorWindow(Editor& aEditor);

		bool Init(const AnimationCurveRef& aCurve);

		void Draw(const float aDeltaTime) override;

		std::string GetName() const override { return "Animation Curve Editor"; }

	private:
		CurveEditor myCurveEditor;
	};
}
