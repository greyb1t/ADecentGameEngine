#pragma once

#include "Curve.h"
#include "CurveList.h"

#include "imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include "Engine/ResourceManagement/ResourceRef.h"

#define CURVE_LINES_COLOR IM_COL32(255, 255, 255, 255)

#define KEY_CIRCLE_COLOR_DEFAULT IM_COL32(255, 255, 255, 255)
#define KEY_CIRCLE_COLOR_HOVER IM_COL32(255, 0, 0, 255)
#define KEY_CIRCLE_COLOR_DRAGGING IM_COL32(255, 255, 255, 255)

#define KEY_TANGENT_CIRCLE_COLOR_DEFAULT IM_COL32(255, 255, 255, 255)
#define KEY_TANGENT_CIRCLE_COLOR_HOVER IM_COL32(255, 0, 0, 255)
#define KEY_TANGENT_CIRCLE_COLOR_DRAGGING IM_COL32(255, 255, 255, 255)
#define KEY_TANGENT_LINE_COLOR IM_COL32(255, 255, 255, 255)

namespace Engine
{
	struct CurveExtended
	{
	};

	class CurveEditor
	{
	public:
		CurveEditor();

		bool Init(const AnimationCurveRef& aCurve);

		void Draw();

	private:
		void DrawCurveWindow(const char* aParentId);
		void DrawControls();
		// void DrawCurveList();

		void UpdateMemberVariables();

		ImVec2 ToWindowSpace(const ImVec2& aValue);

		ImVec2 FromWindowSpace(const ImVec2& aValue);

		ImVec2 FromNormalizedToCurveSpace(const ImVec2& aValue);

		ImVec2 CurveToWindowSpace(const CU::Vector2f& aValue);
		ImVec2 WindowSpaceToCurve(const CU::Vector2f& aValue);

		void UpdateDoubleClickNewKey();

		void UpdateMoveAroundGrid();

		void DrawCurveLines();

		void UpdateCurveKeys();

		bool InvisibleSelectableDraggable(const char* id, const ImVec2& aPos1, const ImVec2& aSize, const CU::Vector2f& aPos2, CU::Vector2f& aOut);

		void DrawGrid();

		void DrawNewCurveModal(const std::string& aModelName);

		void SaveCurve();

		AnimationCurveRef myCurve;
		Curve* mySelectedCurve = nullptr;

		std::string mySelectedCurveName;
		// Required to keep same order when re-ordering keys, then it gets reset after mouse released

		int myHoveredKeyIndex = -1;

		Vec2f myCurvePosition;
		float myMouseWheelValue = 0.f;

		ImGuiWindow* myWindow = nullptr;

		ImVec2 myBottomLeft;
		ImVec2 myTopRight;

		CU::Vector2f myScale = { 0.f, 0.f };

		bool myIsUnsaved = false;
	};
}