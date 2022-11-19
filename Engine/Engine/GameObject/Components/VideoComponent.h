#pragma once

#include "Component.h"
#include "SpriteComponent.h"

namespace Engine
{
	class SpriteMaterialInstance;
	class Sprite;
}

namespace Engine
{
	class CVideo;

	class VideoComponent : public SpriteComponent
	{
		COMPONENT(VideoComponent, "VideoComponent");
	public:
		VideoComponent() = default;
		VideoComponent(GameObject* aGo);

		virtual ~VideoComponent();

		VideoComponent(const VideoComponent& aOther) = default;

		const Vec2ui& GetSize();

		void Reflect(Reflector& aReflector) override;

		void Awake() override;
		void Execute(eEngineOrder aOrder) override;

		void MoveToUIScene();

		void Play(const bool aLoop);
		void Restart();
		void Stop();
		void Pause();
		bool IsFinished() const;
		void SeekToSeconds(const float aSeconds);

		void Finish();

		void SetCallback(const std::function<void()>& aCallbackWhenVideoFinishes);

		float myLoopStartTime = 0.0f;
	private:
		// Renderer::SpriteInstance mySpriteInstance;
		std::string myPath = "";
		Vec2ui myImageSize;
		Shared<CVideo> myVideo;
		bool myIsUI = false;

	};
}