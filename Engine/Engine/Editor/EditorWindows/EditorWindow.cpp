#include "pch.h"
#include "EditorWindow.h"
#include "imgui_internal.h"

void Engine::EditorWindow::UpdateInternal(const float aDeltaTime)
{
	myFeedbackDownTimer -= aDeltaTime;
	myFeedbackDownTimer = std::max(0.f, myFeedbackDownTimer);

	if (myBringToFront)
	{
		myBringToFront = false;

		if (myBringToFrontFocus)
		{
			ImGui::SetNextWindowFocus();
		}

		// https://github.com/ocornut/imgui/issues/2887
		ImGuiWindow* window = ImGui::FindWindowByName(GetName().c_str());

		if (window && window->DockNode && window->DockNode->TabBar)
		{
			window->DockNode->TabBar->NextSelectedTabId = window->ID;
		}

		// TODO: gör så att fönstret dockas by default första gången
		// ImGui::SetNextWindowDockID(ImGui::GetID("MyDockSpace"));
	}

	bool showedFeedback = false;

	if (myFeedbackDownTimer > 0.f)
	{
		showedFeedback = true;

		const auto& defaultTabUnfocusedColor = ImGui::GetStyleColorVec4(ImGuiCol_TabUnfocusedActive);
		const auto& defaultTabActiveColor = ImGui::GetStyleColorVec4(ImGuiCol_TabActive);
		const auto& defaultChildBg = ImGui::GetStyleColorVec4(ImGuiCol_ChildBg);

		const float percent = 1.f - Math::InverseLerp(0.f, ourFeedbackDownDuration, myFeedbackDownTimer);

		const auto lerpedTabActiveColor = Math::Lerp(
			myFeedbackColor,
			Vec4f(),
			percent);

		const auto lerpedTabUnfocusedActiveColor = Math::Lerp(
			myFeedbackColor,
			Vec4f(),
			percent);

		const auto lerpedChildBgColor = Math::Lerp(
			myFeedbackColor,
			Vec4f(),
			percent);

		ImGui::PushStyleColor(ImGuiCol_TabUnfocusedActive, ImVec4(
			defaultTabUnfocusedColor.x + lerpedTabUnfocusedActiveColor.x,
			defaultTabUnfocusedColor.y + lerpedTabUnfocusedActiveColor.y,
			defaultTabUnfocusedColor.z + lerpedTabUnfocusedActiveColor.z,
			defaultTabUnfocusedColor.w + lerpedTabUnfocusedActiveColor.w));

		ImGui::PushStyleColor(ImGuiCol_TabActive, ImVec4(
			defaultTabActiveColor.x + lerpedTabActiveColor.x,
			defaultTabActiveColor.y + lerpedTabActiveColor.y,
			defaultTabActiveColor.z + lerpedTabActiveColor.z,
			defaultTabActiveColor.w + lerpedTabActiveColor.w));

		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(
			defaultChildBg.x + lerpedChildBgColor.x,
			defaultChildBg.y + lerpedChildBgColor.y,
			defaultChildBg.z + lerpedChildBgColor.z,
			defaultChildBg.w + lerpedChildBgColor.w));
	}

	if (Begin(aDeltaTime))
	{
		Draw(aDeltaTime);
	}

	if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
	{
		UpdateFocused(aDeltaTime);

		if (myFlags & Flags::CanBeUnsaved)
		{
			if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_LeftControl)))
			{
				if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_S), false))
				{
					Save();
				}
			}
		}
	}

	if (showedFeedback)
	{
		ImGui::PopStyleColor(3);
	}

	End(aDeltaTime);

	// Don't to anything else here, because we call OnClosed() which can
	// possibly de-allocate this memory
}

void Engine::EditorWindow::BringToFront(bool aFocus)
{
	myBringToFront = true;
	myBringToFrontFocus = aFocus;
}

void Engine::EditorWindow::ShowFeedback()
{
	myFeedbackDownTimer = ourFeedbackDownDuration;
}

void Engine::EditorWindow::MakeUnsaved()
{
	myHasSaved = false;
}

void Engine::EditorWindow::Close()
{
	myIsOpen = false;
}

bool Engine::EditorWindow::IsOpen() const
{
	return myIsOpen;
}

Engine::Editor& Engine::EditorWindow::GetEditor()
{
	return myEditor;
}

bool Engine::EditorWindow::Begin(const float aDeltaTime)
{
	ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);

	ImGuiWindowFlags imguiFlags = GetWindowFlags();

	if (myFlags & Flags::CanBeUnsaved)
	{
		if (!myHasSaved)
		{
			imguiFlags |= ImGuiWindowFlags_UnsavedDocument;
		}
	}

	if (myFlags & Flags::CanBeClosed)
	{
		return ImGui::Begin(GetName().c_str(), &myIsOpen, imguiFlags);
	}
	else
	{
		return ImGui::Begin(GetName().c_str(), nullptr, imguiFlags);
	}
}

void Engine::EditorWindow::End(const float aDeltaTime)
{
	ImGui::End();
}

void Engine::EditorWindow::Save()
{
	myHasSaved = true;
	ShowFeedback();
}
