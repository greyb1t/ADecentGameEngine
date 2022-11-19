#pragma once

class Player;
namespace Engine {
	class VFXComponent;
}

class HealHandler
{
public:
	void Init(Player* aPlayer);
	void Update();

	void Heal();

	void Play();
	void Stop();

	float& HealTimeRef();
private:
	Player* myPlayer;

	float myHealingTime = 0.2f;
	float myHealActiveTimer = 0;

	Engine::VFXComponent* myVFX = nullptr;
};