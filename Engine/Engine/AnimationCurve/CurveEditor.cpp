#include "pch.h"
#include "CurveEditor.h"
#include "Curve.h"
#include "Engine/ResourceManagement/Resources/AnimationCurveResource.h"

#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filewritestream.h"

const float globalMinimumScaleFactor = -0.49f;

Engine::CurveEditor::CurveEditor()
{
	/*
	myCurve.AddKey(Key(CU::Vector2f(0.f, 0.f)));
	Key key(CU::Vector2f(0.3f, 0.5f));
	key.myHasLeftTangent = true;
	key.myTangentLeft = CU::Vector2f(0.2f, 0.5f);

	key.myHasRightTangent = true;
	key.myTangentRight = CU::Vector2f(0.4f, 0.5f);

	myCurve.AddKey(key);

	Key key1(CU::Vector2f(0.5f, 0.5f));
	key1.myHasRightTangent = true;
	key1.myTangentRight = CU::Vector2f(0.6f, 0.5f);

	key1.myHasLeftTangent = true;
	key1.myTangentLeft = CU::Vector2f(0.4f, 0.5f);

	myCurve.AddKey(key1);
	myCurve.AddKey(CU::Vector2f(0.7f, 0.3f));
	myCurve.AddKey(CU::Vector2f(1.f, 1.f));
	*/
}

bool Engine::CurveEditor::Init(const AnimationCurveRef& aCurve)
{
	myCurve = aCurve;

	mySelectedCurve = &myCurve->Get();

	return true;
}

ImVec2 KindaLerpImVec(const ImVec2& aA, const ImVec2& aB, const ImVec2& aT)
{
	return ImVec2(
		CU::Lerp(aA.x, aB.x, aT.x),
		CU::Lerp(aA.y, aB.y, aT.y));
}

ImVec2 KindaInverseLerpImVec(const ImVec2& aA, const ImVec2& aB, const ImVec2& aT)
{
	return ImVec2(
		Math::InverseLerpUnclamped(aA.x, aB.x, aT.x),
		Math::InverseLerpUnclamped(aA.y, aB.y, aT.y));
}

void InvisibleSelectable(const void* id, const ImVec2& aPosition, const ImVec2& aSize)
{
	using namespace ImGui;

	auto old = GetCursorScreenPos();

	SetCursorScreenPos(aPosition - ImVec2(aSize.x * 0.5f, aSize.y * 0.5f));
	PushID(id);
	InvisibleButton("yoo", aSize);
	PopID();

	SetCursorScreenPos(old);
}

void Engine::CurveEditor::Draw()
{
	mySelectedCurve = &myCurve->Get();

	if (ImGui::BeginChild("me vhild", ImVec2(0, 0), true))
	{
		DrawCurveWindow("me vhild");

		ImGui::SameLine();

		DrawControls();
	}
	ImGui::EndChild();
}

void Engine::CurveEditor::DrawCurveWindow(const char* aParentId)
{
	const auto size = ImVec2(ImGui::GetWindowContentRegionWidth() * 0.6f, 0);

	if (ImGui::BeginChildFrame(ImGui::GetID(aParentId), size,
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoScrollbar))
	{
		if (mySelectedCurve == nullptr)
		{
			ImGui::EndChildFrame();
			return;
		}

		if (ImGui::GetIO().MouseWheel != 0 && ImGui::IsWindowHovered())
		{
			myMouseWheelValue = ImGui::GetIO().MouseWheel / 100.f;

			myScale += CU::Vector2f(myMouseWheelValue, myMouseWheelValue);

			myScale.x = std::max(myScale.x, globalMinimumScaleFactor);
			myScale.y = std::max(myScale.y, globalMinimumScaleFactor);
		}

		UpdateMemberVariables();

		UpdateDoubleClickNewKey();

		UpdateMoveAroundGrid();

		UpdateCurveKeys();

		DrawCurveLines();

		DrawGrid();
	}
	ImGui::EndChildFrame();
}

void Engine::CurveEditor::DrawControls()
{
	if (ImGui::BeginChild("dsadas", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.4f, 0)))
	{
		ImGui::Text("%.2f, %.2f", myCurvePosition.x, myCurvePosition.y);

		ImGui::SameLine();

		if (ImGui::Button("Reset position"))
		{
			myCurvePosition = { };
		}

		ImGui::SliderFloat2("Zoom", reinterpret_cast<float*>(&myScale), globalMinimumScaleFactor, 2.f);

		ImGui::SameLine();

		if (ImGui::Button("Reset zoom"))
		{
			myScale = { };
		}

		if (ImGui::Button("Reset curve"))
		{
			mySelectedCurve->Clear();

			mySelectedCurve->AddKey(Key(CU::Vector2f(0.f, 0.f)));
			mySelectedCurve->AddKey(Key(CU::Vector2f(1.f, 1.f)));
		}

		ImGui::Dummy(ImVec2(10, 20));

		ImGui::Separator();

		ImGui::Dummy(ImVec2(10, 20));

		if (ImGui::BeginChild("some new shi", ImVec2(0, 0)))
		{
			// DrawCurveList();

			const std::string newCurveModalName = "Create new curve";

			if (ImGui::Button("New curve"))
			{
				ImGui::OpenPopup(newCurveModalName.c_str());
			}

			DrawNewCurveModal(newCurveModalName);

			ImGui::SameLine();

			if (ImGui::Button("Save"))
			{
				SaveCurve();
			}
		}
		ImGui::EndChild();
	}
	ImGui::EndChild();
}

//void Engine::CurveEditor::DrawCurveList()
//{
//	// Duplicate
//
//	std::vector<const char*> names;
//
//	for (const auto& curve : myCurveList.GetCurves())
//	{
//		names.push_back(curve.first.c_str());
//	}
//
//	static int curre = -1;
//
//	if (ImGui::ListBox("", &curre, names.data(), static_cast<int>(names.size())))
//	{
//		SelectCurve(names[curre]);
//	}
//}

void Engine::CurveEditor::UpdateMemberVariables()
{
	myWindow = ImGui::GetCurrentWindow();

	const auto bb = myWindow->InnerRect;

	const float width = bb.Max.x - bb.Min.x;
	const float height = bb.Max.y - bb.Min.y;

	myBottomLeft = ImVec2(bb.Min.x - width * myScale.x, bb.Max.y + height * myScale.y);
	myTopRight = ImVec2(bb.Max.x + width * myScale.x, bb.Min.y - height * myScale.y);

	// Initialize variables here
	//myBottomLeft = ImVec2(bb.Min.x - width * myScale.x, bb.Max.y + height * myScale.y);
	//myTopRight = ImVec2(bb.Max.x + width * myScale.x, bb.Min.y - height * myScale.y);

	// Resort the keys when user is dragging keys past each other
	if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
	{
		mySelectedCurve->SortKeys();
	}
}

ImVec2 Engine::CurveEditor::ToWindowSpace(const ImVec2& aValue)
{
	return KindaLerpImVec(myBottomLeft, myTopRight, aValue);
}

ImVec2 Engine::CurveEditor::FromWindowSpace(const ImVec2& aValue)
{
	return KindaInverseLerpImVec(myBottomLeft, myTopRight, aValue);
}

ImVec2 Engine::CurveEditor::FromNormalizedToCurveSpace(const ImVec2& aValue)
{
	return ImVec2(myCurvePosition.x, myCurvePosition.y) + aValue;
}

ImVec2 Engine::CurveEditor::CurveToWindowSpace(const CU::Vector2f& aValue)
{
	const auto val = myCurvePosition + aValue;
	return ToWindowSpace({ val.x, val.y });
}

ImVec2 Engine::CurveEditor::WindowSpaceToCurve(const CU::Vector2f& aValue)
{
	auto val = FromWindowSpace({ aValue.x, aValue.y });
	return { val.x - myCurvePosition.x, val.y - myCurvePosition.y };
}

void Engine::CurveEditor::UpdateDoubleClickNewKey()
{
	// Handle double click to create new point
	if (ImGui::IsMouseDoubleClicked(0) && ImGui::IsWindowHovered())
	{
		const auto mousePos = ImGui::GetMousePos();
		const auto resultPos = WindowSpaceToCurve({ mousePos.x, mousePos.y });

		Key key(CU::Vector2f{ resultPos.x, resultPos.y });

		mySelectedCurve->AddKey(key);
	}
}

void Engine::CurveEditor::UpdateMoveAroundGrid()
{
	// Handle the move around the grid with the mouse
	if (ImGui::IsItemActive() || ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
	{
		if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
		{
			const auto dragDelta = ImGui::GetMouseDragDelta();
			ImGui::ResetMouseDragDelta();


			const auto yo = ImVec2(myCurvePosition.x, myCurvePosition.y) + FromWindowSpace(myBottomLeft + dragDelta);
			const auto newPoint = yo;

			myCurvePosition = { newPoint.x, newPoint.y };
		}
	}
}

void Engine::CurveEditor::DrawCurveLines()
{
	const float minT = mySelectedCurve->GetMinimumTime();
	const float maxT = mySelectedCurve->GetMaximumTime();

	const int segmentCount = 1000;

	for (int i = 0; i < segmentCount; ++i)
	{
		const float percent1 = static_cast<float>(i) / static_cast<float>(segmentCount);
		const float t1 = CU::Lerp(minT, maxT, percent1);

		float percent2 = static_cast<float>(i + 1) / static_cast<float>(segmentCount);
		const float t2 = CU::Lerp(minT, maxT, percent2);

		const auto pos1 = mySelectedCurve->EvaluateVec2(t1);
		const auto pos2 = mySelectedCurve->EvaluateVec2(t2);

		// Be able to hover on the line to see the values
		InvisibleSelectable(reinterpret_cast<void*>(i), CurveToWindowSpace(pos1), ImVec2(10.f, 10.f));

		if (ImGui::IsItemActive() || ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("Time: %.2f, Value: %.2f", t1, pos2.y);
		}

		myWindow->DrawList->AddLine(
			CurveToWindowSpace(pos1),
			CurveToWindowSpace(pos2), CURVE_LINES_COLOR, 2.f);
	}
}

void Engine::CurveEditor::UpdateCurveKeys()
{
	auto& keys = mySelectedCurve->GetKeys();

	// Draw the circles on each point and handle dragging them
	for (int i = 0; i < static_cast<int>(keys.size()); ++i)
	{
		auto& key = keys[i];
		const float keyCircleRadius = 6.f;

		ImU32 keyCircleColor = KEY_CIRCLE_COLOR_DEFAULT;

		CU::Vector2f keyPosOut;
		ImGui::PushID(&key);
		if (InvisibleSelectableDraggable("key", CurveToWindowSpace(key.myPosition),
			ImVec2(keyCircleRadius, keyCircleRadius) * 4.f, key.myPosition, keyPosOut))
		{
			keyCircleColor = KEY_CIRCLE_COLOR_DRAGGING;

			key.SetPosition({ keyPosOut.x, keyPosOut.y });
		}
		ImGui::PopID();

		if (ImGui::IsItemActive() || ImGui::IsItemHovered())
		{
			keyCircleColor = KEY_CIRCLE_COLOR_HOVER;

			myHoveredKeyIndex = i;

			// On right click
			ImGui::OpenPopupOnItemClick("EditKey");

			ImGui::SetTooltip("Time: %.2f, Value: %.2f", key.myPosition.x, key.myPosition.y);
		}

		if (i == myHoveredKeyIndex)
		{
			if (ImGui::BeginPopupContextItem("EditKey"))
			{
				ImGui::Text("Time:");
				ImGui::SameLine();
				ImGui::PushID("time");
				ImGui::InputFloat("", reinterpret_cast<float*>(&key.myPosition.x));
				ImGui::PopID();

				ImGui::Text("Value:");
				ImGui::SameLine();
				ImGui::PushID("value");
				ImGui::InputFloat("", reinterpret_cast<float*>(&key.myPosition.y));
				ImGui::PopID();

				if (ImGui::Checkbox("Left tangent", &key.myHasLeftTangent))
				{
					key.myTangentLeft = key.myPosition + CU::Vector2f(-0.05f, 0);
				}

				if (ImGui::Checkbox("Right tangent", &key.myHasRightTangent))
				{
					key.myTangentRight = key.myPosition + CU::Vector2f(0.05f, 0);
				}

				if (ImGui::Button("Delete"))
				{
					mySelectedCurve->RemoveKeyAtIndex(i);
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
		}

		const auto circlePos = CurveToWindowSpace(key.myPosition);

		myWindow->DrawList->AddCircle(circlePos, keyCircleRadius, keyCircleColor);

		const float tangentCircleRadius = 3.f;

		if (key.myHasLeftTangent)
		{
			const auto leftTangentPos = CurveToWindowSpace(key.myTangentLeft);

			myWindow->DrawList->AddLine(circlePos, leftTangentPos, KEY_TANGENT_LINE_COLOR);

			ImU32 tangentColor = KEY_TANGENT_CIRCLE_COLOR_DEFAULT;

			CU::Vector2f out;
			ImGui::PushID(&key);
			if (InvisibleSelectableDraggable("left tang", leftTangentPos,
				ImVec2(keyCircleRadius, keyCircleRadius) * 2.f, key.myTangentLeft, out))
			{
				tangentColor = KEY_TANGENT_CIRCLE_COLOR_DRAGGING;
				key.myTangentLeft = out;
			}
			ImGui::PopID();

			if (ImGui::IsItemActive() || ImGui::IsItemHovered())
			{
				tangentColor = KEY_TANGENT_CIRCLE_COLOR_HOVER;
			}

			myWindow->DrawList->AddCircleFilled(leftTangentPos, tangentCircleRadius, tangentColor);
		}

		if (key.myHasRightTangent)
		{
			const auto rightTangentPosWindowSpace = CurveToWindowSpace(key.myTangentRight);

			myWindow->DrawList->AddLine(circlePos, rightTangentPosWindowSpace, KEY_TANGENT_LINE_COLOR);

			ImU32 tangentColor = KEY_TANGENT_CIRCLE_COLOR_DEFAULT;

			CU::Vector2f out;
			ImGui::PushID(&key);
			if (InvisibleSelectableDraggable("right rang", rightTangentPosWindowSpace,
				ImVec2(keyCircleRadius, keyCircleRadius) * 2.f, key.myTangentRight, out))
			{
				tangentColor = KEY_TANGENT_CIRCLE_COLOR_DRAGGING;
				key.myTangentRight = out;
			}
			ImGui::PopID();

			if (ImGui::IsItemActive() || ImGui::IsItemHovered())
			{
				tangentColor = KEY_TANGENT_CIRCLE_COLOR_HOVER;
			}

			myWindow->DrawList->AddCircleFilled(rightTangentPosWindowSpace, tangentCircleRadius, tangentColor);
		}
	}
}

bool Engine::CurveEditor::InvisibleSelectableDraggable(const char* id, const ImVec2& aPositionInWindowSpace, const ImVec2& aSize, const CU::Vector2f& aPos2, CU::Vector2f& aOut)
{
	InvisibleSelectable(id, aPositionInWindowSpace, aSize);

	if (ImGui::IsMouseDragging(0) && ImGui::IsItemActive())
	{
		const auto dragDelta = ImGui::GetMouseDragDelta();
		ImGui::ResetMouseDragDelta();

		const auto yo = ImVec2(aPos2.x, aPos2.y) +
			FromWindowSpace(myBottomLeft + dragDelta);

		const auto newPoint = ((yo));

		aOut = { newPoint.x, newPoint.y };

		return true;
	}

	return false;
}

void Engine::CurveEditor::DrawGrid()
{
	const auto& windowBounds = myWindow->InnerRect;

	const auto value = WindowSpaceToCurve({ windowBounds.Min.x, windowBounds.Min.y });

	// ImGui::Text("%.2f, %.2f", value.x, value.y);
}

void Engine::CurveEditor::DrawNewCurveModal(const std::string& aModelName)
{
	if (ImGui::BeginPopupModal(aModelName.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		static char nameBuf[255] = { };
		ImGui::InputText("Name", nameBuf, sizeof(nameBuf));

		if (ImGui::Button("Create"))
		{
			Curve curve;
			curve.InitLinear();

			// myCurveList.AddCurve(nameBuf, curve);

			memset(nameBuf, 0, sizeof(nameBuf));

			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void Engine::CurveEditor::SaveCurve()
{
	FileIO::RemoveReadOnly(myCurve->GetPath());

	std::ofstream file(myCurve->GetPath());

	nlohmann::json j;

	j["Keys"] = myCurve->Get().ToJson();

	if (file.is_open())
	{
		file << std::setw(4) << j;
		file.close();
	}
	else
	{
		LOG_ERROR(LogType::Engine) << "Failed to save curve, is it read only?";
	}

	myIsUnsaved = false;
}