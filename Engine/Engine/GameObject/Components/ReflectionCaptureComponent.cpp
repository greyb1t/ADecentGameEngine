#include "pch.h"
#include "ReflectionCaptureComponent.h"
#include "Engine\Reflection\Reflector.h"
#include "Engine\GameObject\GameObject.h"
#include "Engine\Engine.h"
#include "Engine\Renderer\GraphicsEngine.h"
#include "Engine\Renderer\Texture\FullscreenTextureFactory.h"
#include "Engine\Renderer\Texture\Texture2D.h"
#include "Engine\ResourceManagement\Resources\TextureResource.h"
#include "EnvironmentLightComponent.h"
#include "..\DirectXTex\DirectXTex.h"
#include "Engine\Renderer\Directx11Framework.h"
#include "Engine\CopyComponentUsingReflect.h"

void Engine::ReflectionCaptureComponent::Start()
{
}

void Engine::ReflectionCaptureComponent::Execute(eEngineOrder aOrder)
{
}

void Engine::ReflectionCaptureComponent::Render()
{
	const auto& t = myGameObject->GetTransform();

	if (myGeneratedCubemap.GetCubeSRV())
	{
		myGameObject->GetScene()->GetRendererScene().RenderReflectionCapture(*this);
	}

	//GDebugDrawer->DrawCube3D(
	//	DebugDrawFlags::Always,
	//	t.GetPosition(),
	//	t.GetRotation().EulerAngles(),
	//	mySize * t.GetScale());
}

void Engine::ReflectionCaptureComponent::Reflect(Reflector& aReflector)
{
	Component::Reflect(aReflector);

	// 	aReflector.Reflect(mySize, "Size");
	// 
	// 	myMaterialInstance.Reflect(aReflector, 0);

	if (aReflector.Reflect(myGenerateCubemap, "Generate Cubemap") & ReflectorResult_Changed)
	{
		myIsDirty = true;
		myGenerateCubemap = false;

		InitMoney();

		/*
		for (const auto& cam : myGeneratedCubemapCams)
		{
			auto& scene = myGameObject->GetScene()->GetRendererScene();

			GetEngine().GetGraphicsEngine().GetRenderManager().RenderScene(
				scene,
				cam,
				false);

			// myGeneratedCubemap.SetAsActiveDepthSide
			//GetEngine().GetGraphicsEngine().GetRenderManager().EndRender2(scene, );
		}
		*/
	}

	if (aReflector.Reflect(myToggleMainCubemap, "ToggleMainCubemap") & ReflectorResult_Changed)
	{
		if (myGameObject)
		{
			if (myToggleMainCubemap)
			{
				// Turned this cubemap on
				myGeneratedCubemapRefPrev = myGameObject->GetScene()->GetRendererScene().GetEnvironmentalLight()->GetCubemap();

				//myGameObject->GetScene()->GetRendererScene().GetEnvironmentalLight()->SetCubemap1(myGeneratedCubemapRef);

				myGameObject->GetScene()->GetRendererScene().GetEnvironmentalLight()->SetCubemap1(mySmoothCubemapRef);
			}
			else
			{
				if (myGameObject->GetScene()->GetRendererScene().GetEnvironmentalLight() && myGeneratedCubemapRefPrev && myGeneratedCubemapRefPrev->IsValid())
				{
					myGameObject->GetScene()->GetRendererScene().GetEnvironmentalLight()->SetCubemap1(myGeneratedCubemapRefPrev);
				}
			}
		}
	}

	if (aReflector.Button("Save to file"))
	{
		DirectX::ScratchImage image;

		ID3D11Device* device = GetEngine().GetGraphicsEngine().GetDxFramework().GetDevice();
		ID3D11DeviceContext* context = GetEngine().GetGraphicsEngine().GetDxFramework().GetContext();

		// TODO: I must save each side?
		// or each mip?
		// no, the mips should be saved together in the same DDS
		DirectX::CaptureTexture(
			device,
			context,
			myCubemapTextureSmooth.GetTexture2D(),
			image);

		const std::string path = "Assets\\money\\money.dds";
		const std::wstring pathW = L"Assets\\money\\money.dds";

		FileIO::RemoveReadOnly(path);

		HRESULT result = DirectX::SaveToDDSFile(
			image.GetImages(),
			image.GetImageCount(),
			image.GetMetadata(),
			DirectX::DDS_FLAGS_NONE,
			pathW.c_str());

		if (FAILED(result))
		{
			LOG_ERROR(LogType::Resource) << "Failed to save cubemap";
		}
	}
}

Engine::Camera& Engine::ReflectionCaptureComponent::GetCamera(const int aSide)
{
	assert(aSide >= 0 && aSide < 6);
	return myGeneratedCubemapCams[aSide];
}

Engine::FullscreenTextureCube& Engine::ReflectionCaptureComponent::GetCubemapTexture()
{
	return myGeneratedCubemap;
}

Engine::CubemapTexture& Engine::ReflectionCaptureComponent::GetCubemapTextureSmooth()
{
	return myCubemapTextureSmooth;
}

void Engine::ReflectionCaptureComponent::InitMoney()
{
	Vec2ui cubemapRes = { 512, 512 };

	myCubemapTextureSmooth.Init(cubemapRes);
	auto t2 = MakeShared<Texture2D>(
		myCubemapTextureSmooth.GetTexture2D(),
		myCubemapTextureSmooth.GetCubeSRV(),
		cubemapRes);

	// nu måste jag rendera till den också för fan

	mySmoothCubemapRef = MakeShared<TextureResource>(t2);

	// const auto resolution = Vec2ui(128, 128); >

	myGeneratedCubemap = GetEngine().GetGraphicsEngine().GetFullscreenTextureFactory().CreateCubemapTexture(cubemapRes, true);


	auto t = MakeShared<Texture2D>(
		myGeneratedCubemap.GetTexture2D(),
		myGeneratedCubemap.GetCubeSRV(),
		cubemapRes);

	// nu måste jag rendera till den också för fan

	myGeneratedCubemapRef = MakeShared<TextureResource>(t);
	//myCubemap = myGeneratedCubemapRef;

	for (int sideIndex = 0; sideIndex < 6; ++sideIndex)
	{
		Vec3f rot;

		const float deg90 = Math::PI * 0.5f;

		// NOTE: The order for these are VERY important, find the order
		// by searching google "directx 11 cube map faces"
		// and an image pops up from d3d9 that shows it
		// https://docs.microsoft.com/en-us/windows/win32/direct3d9/cubic-environment-mapping
		switch (sideIndex)
		{
		case 0:
			// +X
			rot = Vec3f(0, -deg90, 0);
			break;
		case 1:
			// -X
			rot = Vec3f(0, deg90, 0);
			break;
		case 2:
			// +Y
			rot = Vec3f(deg90, 0, 0);
			break;
		case 3:
			// -Y
			rot = Vec3f(-deg90, 0, 0);
			break;
		case 4:
			// +Z
			rot = Vec3f(0, 0, 0);
			break;
		case 5:
			// -Z
			rot = Vec3f(0, deg90 * 2.f, 0);
			break;
		default:
			assert(false && "unhandled");
			break;
		}


		// Resolution must be same width and height, otherwise its no cube
		// FOV must be 90 to cover an entire cube side
		myGeneratedCubemapCams[sideIndex].InitPerspective(
			90.f, cubemapRes.CastTo<float>(), 1.f, 25000.f);

		myGeneratedCubemapCams[sideIndex].SetRotationEuler(rot);
	}
}
