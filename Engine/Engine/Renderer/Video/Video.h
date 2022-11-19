#pragma once

#include "VideoPlayer.h"
#include "Engine/Renderer/Texture/Texture2D.h"

namespace Engine
{
	class Sprite;
	class ResourceManager;
	class Texture2D;
}

namespace Engine
{
	class CVideoPlayer;

	enum VideoStatus
	{
		VideoStatus_Idle,
		VideoStatus_Playing,
		VideoStatus_ReachedEnd
	};
	class CVideo
	{
	public:
		CVideo();
		~CVideo();
		bool Init(
			ID3D11Device& aDevice, 
			ID3D11DeviceContext& aContext,
			const char* aPath, 
			bool aPlayAudio = false);
		void Play(bool aLoop, float aLoopStartTime);
		void Pause();
		void Stop();
		void SeekToSeconds(const float aSeconds);

		void Update(ID3D11DeviceContext& aContext, float aDelta);

		void Restart();

		Vec2f GetVideoSize() const;
		Vec2ui GetVideoPower2Size() const;

		VideoStatus GetStatus() const;

		Shared<Texture2D> GetTexture();

		std::function<void()> myVideoFinishedCallback;

	private:
		CVideoPlayer myPlayer;

		Vec2f mySize;
		Vec2ui myPower2Size;

		float myUpdateTime = 0.f;
		float myLoopStartTime = 0.0f;
		VideoStatus myStatus = VideoStatus_Idle;
		bool myWantsToPlay = false;
		bool myIsLooping = false;

		Shared<Texture2D> myTexture;

	};
}
