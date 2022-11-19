#include "pch.h"
#include "video.h"
#include "videoplayer.h"
#include "Engine/Engine.h"
#include "..\ResourceManagement\Resources\TextureResource.h"
#include "..\ResourceManagement\ResourceManager.h"
#include "Engine\Renderer\Texture\Texture2D.h"

Engine::CVideo::CVideo()
{
}

Engine::CVideo::~CVideo()
{
}

void Engine::CVideo::Play(bool aLoop, float aLoopStartTime)
{
	myWantsToPlay = true;
	myIsLooping = aLoop;
	myLoopStartTime = aLoopStartTime;
}

void Engine::CVideo::Pause()
{
	myWantsToPlay = false;
}

void Engine::CVideo::Stop()
{
	myStatus = VideoStatus_Idle;
	myWantsToPlay = false;
	myPlayer.Stop();
}

void Engine::CVideo::SeekToSeconds(const float aSeconds)
{
	myWantsToPlay = true;
	myStatus = VideoStatus_Playing;

	myPlayer.SeekToSeconds(aSeconds);
}

void Engine::CVideo::Restart()
{
	myWantsToPlay = true;
	myPlayer.RestartStream(myLoopStartTime);
	myPlayer.SeekToSeconds(myLoopStartTime);
}

Vec2f Engine::CVideo::GetVideoSize() const
{
	return mySize;
}

Vec2ui Engine::CVideo::GetVideoPower2Size() const
{
	return myPower2Size;
}

Engine::VideoStatus Engine::CVideo::GetStatus() const
{
	return myStatus;
}

Shared<Engine::Texture2D> Engine::CVideo::GetTexture()
{
	return myTexture;
}

bool Engine::CVideo::Init(
	ID3D11Device& aDevice,
	ID3D11DeviceContext& aContext,
	const char* aPath,
	bool aPlayAudio)
{
	EVideoError error = myPlayer.Init(aPath, aPlayAudio);
	if (error == EVideoError_WrongFormat)
	{
		LOG_ERROR(LogType::Audio) << "Could not load video: " << aPath << ". Wrong format?";
		return false;
	}
	else if (error == EVideoError_FileNotFound)
	{
		LOG_ERROR(LogType::Audio) << "Could not load video: " << aPath << ". File not found";
		return false;
	}

	if (!myPlayer.DoFirstFrame())
	{
		LOG_ERROR(LogType::Audio) << "Could not load video: " << aPath << ". First frame not found";
		return false;
	}

	mySize.x = myPlayer.GetAvVideoFrame()->width;
	mySize.y = myPlayer.GetAvVideoFrame()->height;

	//myPower2Size.x = (int)powf(2, ceilf(logf((float)mySize.x) / logf(2)));
	//myPower2Size.y = (int)powf(2, ceilf(logf((float)mySize.y) / logf(2)));

	myPower2Size.x = mySize.x;
	myPower2Size.y = mySize.y;

	myStatus = VideoStatus_Playing;

	D3D11_TEXTURE2D_DESC texture_desc;
	texture_desc.Width = myPower2Size.x;
	texture_desc.Height = myPower2Size.y;
	texture_desc.MipLevels = 1;
	texture_desc.ArraySize = 1;
	texture_desc.MipLevels = 1;
	texture_desc.ArraySize = 1;

	texture_desc.SampleDesc.Count = 1;
	texture_desc.SampleDesc.Quality = 0;
	texture_desc.Usage = D3D11_USAGE_DYNAMIC;
	texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	texture_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	texture_desc.MiscFlags = 0;

	ComPtr<ID3D11Texture2D> texture2d;
	ComPtr<ID3D11ShaderResourceView> srv;

	aDevice.CreateTexture2D(&texture_desc, nullptr, &texture2d);
	aDevice.CreateShaderResourceView(texture2d.Get(), NULL, &srv);

	myTexture = MakeShared<Texture2D>(texture2d, srv, myPower2Size);

	// Force one frame so we dont get a blank video
	bool wantsToPlay = myWantsToPlay;
	myWantsToPlay = true;
	myUpdateTime = 0.0001f;
	Update(aContext, 0);
	myUpdateTime = 0.0f;
	myWantsToPlay = wantsToPlay;

	return true;
}

void Engine::CVideo::Update(ID3D11DeviceContext& aContext, float aDelta)
{
	ZoneNamedN(zone1, "CVideo::Update", true);

	if (!myWantsToPlay)
	{
		return;
	}
	myUpdateTime += aDelta;

	double fps = myPlayer.GetFps();

	{
		ZoneNamedN(zone3, "CVideo::Update-(Iteration Part)", true);

		while (myUpdateTime >= 1.0f / fps)
		{
			if (myTexture->GetSRV())
			{
				int status = myPlayer.GrabNextFrame();

				if (status < 0)
				{
					myStatus = VideoStatus_ReachedEnd;
					
					if (myVideoFinishedCallback)
					{
						myVideoFinishedCallback();
					}

					if (myIsLooping)
					{
						Restart();
					}

				}

				//D3D11_MAPPED_SUBRESOURCE  mappedResource;

				//HRESULT result = aContext.Map(myTexture->GetTexture2D(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

				//if (FAILED(result))
				//{
				//	return;
				//}

				//unsigned int* source = (unsigned int*)(mappedResource.pData);

				//myPlayer.ConvertVideoToArray(source, myPower2Size.x, myPower2Size.y);

				//aContext.Unmap(myTexture->GetTexture2D(), 0);
			}
			myUpdateTime -= 1.0f / static_cast<float>(fps);
		}
	}

	{
		ZoneNamedN(zone2, "CVideo::Update-(Map Part)", true);

		D3D11_MAPPED_SUBRESOURCE  mappedResource;

		HRESULT result = aContext.Map(myTexture->GetTexture2D(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

		if (FAILED(result))
		{
			return;
		}

		unsigned int* source = (unsigned int*)(mappedResource.pData);

		myPlayer.ConvertVideoToArray(source, myPower2Size.x, myPower2Size.y);

		aContext.Unmap(myTexture->GetTexture2D(), 0);
	}
}

