#include "pch.h"
#include "StatsDisplay.h"
#include "Engine/Engine.h"
#include "Player/Player.h"
#include "Components/HealthComponent.h"
#include "Engine/GameObject/GameObject.h"

void StatsDisplay::Update()
{
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Tools"))
		{
			if (ImGui::MenuItem("Player Stats", "Ctrl+K", myIsDisplaying))
			{
				ToggleMenu();
			}

			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}

	if (GetEngine().GetInputManager().IsKeyPressed(Common::KeyCode::LeftControl) && 
		GetEngine().GetInputManager().IsKeyDown(Common::KeyCode::K))
	{
		ToggleMenu();
	}

	if (myIsDisplaying && Main::GetPlayer())
	{
		GameObject* playerObj = Main::GetPlayer();
		Player* player = playerObj->GetComponent<Player>();
		HealthComponent* health = playerObj->GetComponent<HealthComponent>();
		if (!player || !health)
		{
			return;
		}
		if (ImGui::Begin("PlayerStats"))
		{
			ImGui::Text("Damage: ");
			ImGui::SameLine();
			ImGui::Text(std::to_string(player->GetStats().GetDamage()).c_str());

			ImGui::Text("Attack speed: ");
			ImGui::SameLine();
			ImGui::Text(std::to_string(player->GetStats().GetAttackSpeed()).c_str());

			ImGui::Text("Move speed: ");
			ImGui::SameLine();
			ImGui::Text(std::to_string(player->GetStats().GetMovementSpeed()).c_str());

			ImGui::Text("Jump force: ");
			ImGui::SameLine();
			ImGui::Text(std::to_string(player->GetStats().GetJumpForce()).c_str());

			ImGui::Text("Jump times: ");
			ImGui::SameLine();
			ImGui::Text(std::to_string(player->GetStats().GetJumpAmount()).c_str());

			ImGui::Text("Cooldown multiplier: ");
			ImGui::SameLine();
			ImGui::Text(std::to_string(player->GetStats().GetCooldownMultiplier()).c_str());

			ImGui::Text("Crit chance ");
			ImGui::SameLine();
			ImGui::Text(std::to_string(player->GetStats().GetCritChance()).c_str());

			ImGui::Text("Crit multiplier ");
			ImGui::SameLine();
			ImGui::Text(std::to_string(player->GetStats().GetCritDamageMultiplier()).c_str());

			ImGui::Text("Max HP: ");
			ImGui::SameLine();
			ImGui::Text(std::to_string(health->GetMaxHealth()).c_str());

			ImGui::Text("Shield: ");
			ImGui::SameLine();
			ImGui::Text(std::to_string(health->GetShield()).c_str());
		}
		ImGui::End();
	}
	else
	{
		myIsDisplaying = false;
	}
}

void StatsDisplay::ToggleMenu()
{
	if (Main::GetPlayer())
	{
		myIsDisplaying = !myIsDisplaying;
	}
}
