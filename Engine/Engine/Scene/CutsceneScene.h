#pragma once

#include "Scene.h"
#include "Engine/Utils/DownTimer.h"

namespace Engine
{
	class VideoComponent;
	class SpriteComponent;
	class Curve;

	class CutsceneScene : public Scene
	{
	public:
		bool Init(
			const Path& aCutscenePath,
			const std::string& aAudioEventPath,
			std::function<void()> aOnFinished);

		void Update(TimeStamp ts) override;

	private:
		void UpdateSkipSpriteFade(TimeStamp ts);

	private:
		enum class FadeStatus
		{
			FadeIn,
			FadeOut,
			Nothing
		};

		GameObject* myVideoGameObject = nullptr;
		VideoComponent* myVideoComponent = nullptr;

		GameObject* mySkipSpriteGameObject = nullptr;
		SpriteComponent* mySkipSpriteComponent = nullptr;

		C::Vector4f myTransparentColor;
		C::Vector4f myWhiteColor;

		std::function<void()> myOnFinished;

		const Curve* mySkipFadeInCurve = nullptr;
		const Curve* mySkipFadeOutCurve = nullptr;

		FadeStatus mySkipSpriteFadeStatus = FadeStatus::Nothing;
		DownTimer mySkipFadeDownTimer;
	};
}