#pragma once

#include "Engine/GameObject/Components/Component.h"
#include "Engine/UIEventManager/UIEventManager.h"

namespace Engine
{
	BETTER_ENUM(eMusicLevel, int,

		GRASSLANDS = 0,
		DESERT,
		MINIBOSS,
		FINALBOSS,
		MAINMENU,
		COUNT
	);
	class MusicManager : public Component
	{
	public:
		COMPONENT_SINGLETON(MusicManager, "MusicAndAMBManager");

		MusicManager() = default;
		MusicManager(GameObject* aGameObject);

		virtual ~MusicManager();

		void Start() override;
		void Execute(eEngineOrder aOrder) override;
		void Reflect(Reflector& aReflector) override;
		
		void Play();
		void PlayDelayed(float aTime);
		void Stop();
		void SetParameter(const std::string& aParameterName, float aValue);
		void SetParameter(int aUUID, const std::string& aParameterName, float aValue);
		void SetIndividualParameter(int aUUID, const std::string& aEvent, const std::string& aParameterName, float aValue);
		void SetLevel(eMusicLevel aLevel);

		void SetNumEnemies(const float& aValue);

		eMusicLevel GetLevel();

		void ActivateMiniBoss();
		void ActivateFinalBoss();
		void HandleMusicParameters();
		void HandleDelayedPlay();

	private:
		void PlayMusic();
		eMusicLevel myLevel = eMusicLevel::COUNT;
		float myParameterValue = -1.0f;
		float myPreviousParameterValue = -1.0f;
		float myNumEnemies = -1.0f;

		float myDelayTimer = 0.0f;
		float myDelayTime = 0.0f;
		bool myDelayTimerOn = false;

		bool myShouldPlay = true; 
		bool myTriggerFinalBoss = false;
	};
}