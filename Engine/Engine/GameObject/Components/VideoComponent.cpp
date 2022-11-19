#include "pch.h"
#include "VideoComponent.h"
#include "..\Engine.h"
#include "..\GameObject.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Renderer/GraphicsEngine.h"
#include "Engine/Renderer/Scene/RendererScene.h"
#include "Engine\Renderer\Material\MaterialFactory.h"
#include "Engine/Renderer\Material\MeshMaterial.h"
#include "Engine/Renderer\Video\Video.h"
#include "Engine/Renderer\Directx11Framework.h"
#include "Engine/ResourceManagement\Resources\TextureResource.h"
#include "Engine\Renderer\Texture\Texture2D.h"

Engine::VideoComponent::VideoComponent(GameObject* aGo) : SpriteComponent(aGo)
{
}

Engine::VideoComponent::~VideoComponent()
{
}

void Engine::VideoComponent::Awake()
{
	if (myPath == "")
		return;

	myVideo = MakeShared<CVideo>();

	if (!myVideo->Init(
		*GetEngine().GetGraphicsEngine().GetDxFramework().GetDevice(),
		*GetEngine().GetGraphicsEngine().GetDxFramework().GetContext(),
		myPath.c_str()))
	{
		myVideo = nullptr;
		LOG_ERROR(LogType::Engine) << "File path was wrong for video component!";
		return;
	}

	// mySprite = GetEngine().GetGraphicsEngine().GetSpriteFactory().CreateSpriteWithoutTexture();

	auto texture = myVideo->GetTexture();

	TextureRef lol = MakeShared<TextureResource>(texture);

	// mySprite->GetData().myTexture = lol;

	auto videoSize = myVideo->GetVideoSize();

	// mySprite->GetData().myTextureSize.x = videoSize.x;
	// mySprite->GetData().myTextureSize.y = videoSize.y;

	const auto videoPower2Size = myVideo->GetVideoPower2Size().CastTo<float>();

	float uvSizeX = videoSize.x / videoPower2Size.x;
	float uvSizeY = videoSize.y / videoPower2Size.y;

	SetSprite(lol);

	UVRect uvRect;
	{
		uvRect.myStart.x = 0.f;
		uvRect.myStart.y = 0.f;
		uvRect.myEnd.x = uvSizeX;
		uvRect.myEnd.y = uvSizeY;
	}

	myUVRect = uvRect;
	// <
	myVideo->Play(true, myLoopStartTime);

	mySortOrder = mySortOrder;

	myImageSize = myTexture->Get().GetImageSize();
}

void Engine::VideoComponent::Execute(eEngineOrder aOrder)
{
	ZoneScopedN("VideoComponent::Execute");

	if (!myVideo)
		return;

	const auto& transform = myGameObject->GetTransform();

	// mySpriteInstance.GetTransform().SetPosition(transform.GetPositionWorld());
	// mySpriteInstance.GetTransform().SetRotationQuaternion(transform.GetRotationWorld());
	// mySpriteInstance.GetTransform().SetScale(transform.GetScaleWorld());

	myVideo->Update(*GetEngine().GetGraphicsEngine().GetDxFramework().GetContext(), Time::DeltaTimeUnscaled);

	if (myIsUI)
	{
		myGameObject->GetScene()->GetUIRendererScene().RenderSprite(*this);
	}
	else
	{
		myGameObject->GetScene()->GetRendererScene().RenderSprite(*this);
	}
}

void Engine::VideoComponent::MoveToUIScene()
{
	myIsUI = true;
}

void Engine::VideoComponent::Play(const bool aLoop)
{
	if (myVideo)
	{
		myVideo->Play(aLoop, myLoopStartTime);
	}
}

void Engine::VideoComponent::Restart()
{
	myVideo->Restart();
}

void Engine::VideoComponent::Stop()
{
	myVideo->Stop();
}

void Engine::VideoComponent::Pause()
{
	myVideo->Pause();
}

bool Engine::VideoComponent::IsFinished() const
{
	return myVideo->GetStatus() == VideoStatus_ReachedEnd;
}

void Engine::VideoComponent::SeekToSeconds(const float aSeconds)
{
	myVideo->SeekToSeconds(aSeconds);
}

void Engine::VideoComponent::Finish()
{
	myVideo->Stop();

	if (myVideo->myVideoFinishedCallback)
	{
		myVideo->myVideoFinishedCallback();
	}
}

void Engine::VideoComponent::SetCallback(const std::function<void()>& aCallbackWhenVideoFinishes)
{
	if (myVideo)
	{
		myVideo->myVideoFinishedCallback = aCallbackWhenVideoFinishes;
	}
}

const Vec2ui& Engine::VideoComponent::GetSize()
{
	return myImageSize;
}

#include "Engine/Reflection/Reflector.h"
void Engine::VideoComponent::Reflect(Reflector& aReflector)
{
	SpriteComponent::Reflect(aReflector);
	aReflector.Reflect(myPath, "Video Path");
	aReflector.Reflect(myLoopStartTime, "Loop start time");

	if (aReflector.Button("Play Video"))
	{
		Play(true);
	}

	if (aReflector.Button("Play From Loop Start Time"))
	{
		myVideo->SeekToSeconds(myLoopStartTime);
	}
}
