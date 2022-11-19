#include "pch.h"
#include "DebugMenu.h"
#include "..\Engine.h"

namespace Engine
{
void DebugMenu::RenderImgui()
{
#ifndef _RETAIL
	if (Enabled())
	{
		ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);

		if (ImGui::Begin("Debug menu"))
		{
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
				1000.0f / ImGui::GetIO().Framerate,
				ImGui::GetIO().Framerate);

			ImGui::Text("FrameTime: %.3f ms", GetEngine().GetTimer().GetDeltaTime() * 1000.f);

			ImGui::Dummy(ImVec2(0, 10));

			for (const auto& menuEntryDrawCall : ourDebugEntryCallbacks)
			{
				ImGui::PushID(menuEntryDrawCall.myId);
				if (ImGui::CollapsingHeader(menuEntryDrawCall.myName.c_str()))
				{
					menuEntryDrawCall.myCallback();
				}
				ImGui::PopID();
			}
		}
		ImGui::End();
	}

	ourDebugEntryCallbacks.clear();
#endif
}

void DebugMenu::AddMenuEntry(
	const std::string& aMenuName, DebugMenuEntryCallback aImguiDrawCallback, void* aId)
{
#ifndef _RETAIL
	DebugMenuEntry entry;
	{
		entry.myCallback = aImguiDrawCallback;
		entry.myName = aMenuName;
		entry.myId = aId;
	}

	ourDebugEntryCallbacks.push_back(entry);
#endif
}

void DebugMenu::ToggleDebugMenu(const bool aEnabled)
{
	ourIsDebugMenuActive = aEnabled;
}

bool DebugMenu::Enabled()
{
	return ourIsDebugMenuActive;
}
}