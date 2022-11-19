#include "pch.h"
#include "DebugDrawerSettings.h"

#include "DebugMenu.h"
#include "Engine/Engine.h"

std::string Engine::DebugDrawFlagToString(const DebugDrawFlags aFlag)
{
	switch (aFlag)
	{
	case DebugDrawFlags::Pointlights:
		return "Pointlights";
	case DebugDrawFlags::Spotlights:
		return "Spotlight";
	case DebugDrawFlags::BoundingSpheres:
		return "BoundingSpheres";
	case DebugDrawFlags::Navmesh:
		return "Navmesh";
	case DebugDrawFlags::Frustum:
		return "Frustum";
	case DebugDrawFlags::EnvironmentLight:
		return "EnvironmentLight";
	case DebugDrawFlags::UI:
		return "UI";
	case DebugDrawFlags::Physics:
		return "Physics";
	case DebugDrawFlags::Particles:
		return "Particles";
	case DebugDrawFlags::Gameplay:
		return "Gameplay";
	case DebugDrawFlags::AI:
		return "AI";
	case DebugDrawFlags::Triggers:
		return "Triggers";
	case DebugDrawFlags::Bezier:
		return "Bezier";
	case DebugDrawFlags::Player:
		return "Player";
	default:
		return "Add case in DebugDrawFlagToString() plz";
		break;
	}
}

void Engine::DebugDrawerSettings::Update()
{
	if (GetEngine().GetInputManager().IsKeyDown(C::KeyCode::F2))
	{
		myActive = !myActive;
	}
}

void Engine::DebugDrawerSettings::DrawDebugMenu()
{
	DebugMenu::AddMenuEntry("DebugDrawer",
		[this]()
		{
			if (ImGui::Checkbox("Enabled", &myActive))
			{
				SaveDebugDrawSettings();
			}

			static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg
				| ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable
				| ImGuiTableFlags_Hideable;

			bool clickedEditTexture = false;

			if (ImGui::BeginTable("debugdrawflags", 1, flags))
			{
				ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableHeadersRow();

				for (int i = 0; i < static_cast<int>(DebugDrawFlags::Count); ++i)
				{
					const auto debugDrawFlag = static_cast<DebugDrawFlags>(i);

					ImGui::PushID(i);

					ImGui::TableNextRow();

					for (int column = 0; column < 1; column++)
					{
						ImGui::TableSetColumnIndex(column);

						switch (column)
						{
						case 0:
						{
							bool isDebugFlagEnabled = IsDebugFlagSet(debugDrawFlag);

							if (ImGui::Checkbox(DebugDrawFlagToString(debugDrawFlag).c_str(),
								&isDebugFlagEnabled))
							{
								SetDebugFlag(debugDrawFlag, isDebugFlagEnabled);

								SaveDebugDrawSettings();
							}
						}
						break;
						default:
							assert(false);
							break;
						}
					}

					ImGui::PopID();
				}
				ImGui::EndTable();
			}

			ImGui::Separator();
		});
}

bool Engine::DebugDrawerSettings::IsDebugFlagSet(const DebugDrawFlags aFlag) const
{
	return myFlags.test(static_cast<int>(aFlag));
}

void Engine::DebugDrawerSettings::SetDebugFlag(const DebugDrawFlags aFlag, const bool aEnabled)
{
	myFlags.set(static_cast<int>(aFlag), aEnabled);
}

bool Engine::DebugDrawerSettings::IsDebugFlagActive(const DebugDrawFlags aFlag) const
{
	if (!IsActive())
	{
		return false;
	}

	return IsDebugFlagSet(aFlag);
}

bool Engine::DebugDrawerSettings::IsActive() const
{
	return myActive;
}

bool Engine::DebugDrawerSettings::LoadDebugDrawSettings()
{
	const std::string path = "Dev/DebugDrawSettings.json";
	std::ifstream file(path);

	if (!file.is_open())
	{
		std::cout << "Failed to open " << path << std::endl;
		return false;
	}

	const bool ignoreComments = true;
	const bool allowExceptions = false;
	nlohmann::json j = nlohmann::json::parse(file, nullptr, allowExceptions, ignoreComments);

	// If failed to parse json file
	if (j.is_discarded())
	{
		std::cout << "Failed to parse " << path << std::endl;
		return false;
	}

	for (int i = 0; i < static_cast<int>(DebugDrawFlags::Count); ++i)
	{
		const auto debugDrawFlag = static_cast<DebugDrawFlags>(i);

		if (j["DebugDrawFlags"].contains(DebugDrawFlagToString(debugDrawFlag)))
		{
			SetDebugFlag(debugDrawFlag, j["DebugDrawFlags"][DebugDrawFlagToString(debugDrawFlag)]);
		}
	}

	return true;
}

bool Engine::DebugDrawerSettings::SaveDebugDrawSettings()
{
	nlohmann::json j;

	j["DebugDrawFlags"] = nlohmann::json::object();

	for (int i = 0; i < static_cast<int>(DebugDrawFlags::Count); ++i)
	{
		const auto debugDrawFlag = static_cast<DebugDrawFlags>(i);

		j["DebugDrawFlags"][DebugDrawFlagToString(debugDrawFlag)] = IsDebugFlagSet(debugDrawFlag);
	}

	const std::string path = "Dev/DebugDrawSettings.json";

	std::ofstream file(path);

	file << std::setw(4) << j;

	file.close();

	return true;
}