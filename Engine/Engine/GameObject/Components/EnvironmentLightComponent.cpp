#include "pch.h"
#include "EnvironmentLightComponent.h"
#include "Engine/Engine.h"
#include "Engine/Renderer/GraphicsEngine.h"
#include "Engine/Renderer/Texture/FullscreenTextureFactory.h"
#include "Engine/ResourceManagement/ResourceManager.h"
#include "Engine/ResourceManagement/Resources/TextureResource.h"
#include "Engine/Reflection/Reflector.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/Renderer/Texture/Texture2D.h"
#include "Engine/Renderer/RenderManager.h"
#include "CameraComponent.h"

Engine::EnvironmentLightComponent::EnvironmentLightComponent(
	GameObject* aGameObject, const std::string& aCubemapPath)
	: Component(aGameObject)
{
	//myCubemap = GResourceManager->CreateRef<TextureResource>(aCubemapPath);
	//myCubemap->Load();
	//
	//if (!myCubemap->IsValid())
	//{
	//	assert(false);
	//}
	//
	//InitLight();
}

void Engine::EnvironmentLightComponent::InitLight()
{
	if (/*aShouldCastShadows*/ true)
	{
		//const auto resolution = CU::Vector2ui(2048, 2048);
		const auto resolution = Vec2ui(ShadowMapSize, ShadowMapSize);

		// NOTE: NEVER 8K TEXTURE, SOME COMPUTER LAG HARD AS FUCK
		//const auto resolution = CU::Vector2ui(8192, 8192);

		myShadowCamera.InitOrthographic(
			resolution.CastTo<float>(), 1.f, 25000.f);

		// environmentalLight.myShadowCamera.SetTransform(aTransform);

		myShadowMap =
			GetEngine().GetGraphicsEngine().GetFullscreenTextureFactory().CreateShadowMap(resolution);


		for (int cascadeIndex = 0; cascadeIndex < NumCascades; ++cascadeIndex)
		{
			myCascadeShadowMaps[cascadeIndex] = GetEngine().GetGraphicsEngine().GetFullscreenTextureFactory().CreateShadowMap(resolution);
		}
	}
}

void Engine::EnvironmentLightComponent::Execute(eEngineOrder aOrder)
{
	//myShadowCamera.SetPosition(myGameObject->GetTransform().GetPosition());
	myShadowCamera.SetRotationQuaternion(myGameObject->GetTransform().GetRotation());
}

void Engine::EnvironmentLightComponent::Render()
{
	

	//std::string sceneName = myGameObject->GetScene()->GetName();
	if(myUpdateCascades)
		CalculateCascades();

	//GDebugDrawer->DrawLine3D(DebugDrawFlags::Always, GetTransform().GetPosition(), GetTransform().GetPosition() + GetDirection().ToVec3() * 10000.f);
	//
	//std::string sceneName = myGameObject->GetScene()->GetName();
	//ImGui::Begin("Cascade settings");
	//ImGui::Checkbox(("Update cascades" + sceneName).c_str(), &myUpdateCascades);
	//for (int i = 0; i < NumCascades; ++i)
	//{
	//	ImGui::DragFloat(("Cascade" + std::to_string(i)).c_str(), &myCascadeBounds[i], 0.001f, 0.f, 1.f);
	//}
	//ImGui::End();
}

void Engine::EnvironmentLightComponent::Reflect(Reflector& aReflector)
{
	Component::Reflect(aReflector);

	aReflector.Header("Cubemap");

	if (aReflector.Reflect(myCubemap, "Cubemap") & (ReflectorResult_Changed | ReflectorResult_Changing))
	{
		InitLight();
	}

	// aReflector.Reflect(myCubemap2, "Cubemap2");
	// aReflector.Reflect(myCubemapInterpolationValue, "Cubemap Interpolation");

	aReflector.SetNextItemRange(0.001f, 100.f);
	aReflector.SetNextItemSpeed(0.01f);
	aReflector.Reflect(myCubemapIntensity, "Cubemap Intensity");

	aReflector.Header("Directional Light");

	aReflector.Reflect(myColor, "Color", ReflectionFlags_IsColor);

	aReflector.SetNextItemRange(0.001f, 100.f);
	aReflector.SetNextItemSpeed(0.01f);
	aReflector.Reflect(myColorIntensity, "Color Intensity");

	aReflector.Reflect(myFolliageCullingDistance, "Folliage Culling Distance");
}

Vec4f Engine::EnvironmentLightComponent::GetDirection() const
{
	return C::Vector4f(
		myGameObject->GetTransform().GetMatrix().GetForward().x,
		myGameObject->GetTransform().GetMatrix().GetForward().y,
		myGameObject->GetTransform().GetMatrix().GetForward().z,
		0.f);
}

Engine::Camera& Engine::EnvironmentLightComponent::GetCamera()
{
	return myShadowCamera;
}

Engine::FullscreenTexture& Engine::EnvironmentLightComponent::GetShadowMap()
{
	return myShadowMap;
}

TextureRef Engine::EnvironmentLightComponent::GetCubemap()
{
	return myCubemap;
}

TextureRef Engine::EnvironmentLightComponent::GetCubemap2()
{
	return myCubemap2;
}

bool Engine::EnvironmentLightComponent::IsCastingShadows() const
{
	return myIsCastingShadows;
}

void Engine::EnvironmentLightComponent::SetCubemapInterpolationValue(const float aValue)
{
	myCubemapInterpolationValue = aValue;
}

float Engine::EnvironmentLightComponent::GetCubemapInterpolationValue() const
{
	return myCubemapInterpolationValue;
}

const int Engine::EnvironmentLightComponent::GetNumCascades() const
{
	return NumCascades;
}

const Mat4f& Engine::EnvironmentLightComponent::GetCascadeProjection(const int aCascadeIndex) const
{
	return myCascadeProjections[aCascadeIndex];
}

const Mat4f& Engine::EnvironmentLightComponent::GetCascadeView(const int aCascadeIndex) const
{
	return myCascadeViews[aCascadeIndex];
}

const float Engine::EnvironmentLightComponent::GetCascadeBound(const int aCascadeIndex) const
{
	return myCascadeBounds[aCascadeIndex];
}

const Engine::EnvironmentLightComponent::CascadeData& Engine::EnvironmentLightComponent::GetCascadeData(const int aCascadeIndex) const
{
	return myCascadeData[aCascadeIndex];
}

Engine::FullscreenTexture& Engine::EnvironmentLightComponent::GetCascadeTexture(const int aCascadeIndex)
{
	return myCascadeShadowMaps[aCascadeIndex];
}

Mat4f Engine::EnvironmentLightComponent::GetShadowView()
{
	return myShadowCamera.GetTransform().ToMatrix();
}

float Engine::EnvironmentLightComponent::GetFolliageCullingDistance() const
{
	return myFolliageCullingDistance;
}

void Engine::EnvironmentLightComponent::CalculateCascades()
{
	for (int cascadeIndex = 0; cascadeIndex < NumCascades; ++cascadeIndex)
	{
		auto& camera = myGameObject->GetScene()->GetMainCamera();

		float nearFarRange = camera.GetFarPlane() - camera.GetNearPlane();
		float nearRange, farRange;
		if (cascadeIndex == 0)
			nearRange = 1.f;
		else
			nearRange = myCascadeBounds[cascadeIndex - 1] * nearFarRange;
		farRange = myCascadeBounds[cascadeIndex] * nearFarRange;

		Mat4f projection;
		projection.InitPerspective(camera.GetFovDegrees(), camera.GetResolution(), nearRange, farRange);

		std::vector<Vec4f> corners = GetFrustumCornersWorldSpace(projection, camera.GetTransform().ToMatrix());

		Vec4f center{};
		for (const auto& v : corners)
		{
			//GDebugDrawer->DrawSphere3D(DebugDrawFlags::Always, v.ToVec3(), 100.0f, 0.0f, Vec4f(0, 1, 0, 1));
			center += v;
		}
		center /= (float)corners.size();

		auto dir = GetDirection();
		const Vec4f lightPos = center - (GetDirection() * nearFarRange);

		Mat4f lightView = Mat4f::LookAt(lightPos.ToVec3(), center.ToVec3());

		for (auto& v : corners)
		{
			v = v * lightView;
		}

		float minX, minY, minZ, maxX, maxY, maxZ;
		minX = minY = minZ = std::numeric_limits<float>::max();
		maxX = maxY = maxZ = -std::numeric_limits<float>::max();

		for (const auto& v : corners)
		{
			minX = std::min(minX, v.x);
			maxX = std::max(maxX, v.x);
			minY = std::min(minY, v.y);
			maxY = std::max(maxY, v.y);
			minZ = std::min(minZ, v.z);
			maxZ = std::max(maxZ, v.z);
		}

		float distZ = maxZ - minZ;
		Mat4f lightProjection = Mat4f::CreateOrthographic(minX, maxX, minY, maxY, 0.1f, (distZ + nearFarRange));
		//Mat4f lightProjection = Mat4f::CreateOrthographic(minX, maxX, minY, maxY, minZ, maxZ);

		myCascadeViews[cascadeIndex] = lightView;
		myCascadeProjections[cascadeIndex] = lightProjection;

		CascadeData data;
		data.nearPlane = 0.1f;
		data.farPlane = distZ + nearFarRange;
		data.resolution = { maxX - minX, maxY - minY };
		myCascadeData[cascadeIndex] = data;
	}
}

std::vector<Vec4f> Engine::EnvironmentLightComponent::GetFrustumCornersWorldSpace(const Mat4f& aProjection, const Mat4f& aView)
{
	std::vector<Vec4f> frustumCorners;

	Mat4f inverseProjection = aProjection.Inverse();
	inverseProjection = inverseProjection * aView;

	frustumCorners.emplace_back(Vec4f{ -1, -1, 0.f, 1 } *inverseProjection);
	frustumCorners.emplace_back(Vec4f{ 1, -1, 0.f, 1 } *inverseProjection);
	frustumCorners.emplace_back(Vec4f{ -1, 1, 0.f, 1 } *inverseProjection);
	frustumCorners.emplace_back(Vec4f{ 1, 1, 0.f, 1 } *inverseProjection);

	frustumCorners.emplace_back(Vec4f{ -1, -1, 1.f, 1 } *inverseProjection);
	frustumCorners.emplace_back(Vec4f{ 1, -1, 1.f, 1 } *inverseProjection);
	frustumCorners.emplace_back(Vec4f{ -1, 1, 1.f, 1 } *inverseProjection);
	frustumCorners.emplace_back(Vec4f{ 1, 1, 1.f, 1 } *inverseProjection);

	for (auto& v : frustumCorners)
	{
		v /= v.w;
	}

	return frustumCorners;
}

void Engine::EnvironmentLightComponent::SetCubemap1(TextureRef aTexture)
{
	myCubemap = aTexture;
}

void Engine::EnvironmentLightComponent::SetCubemap2(TextureRef aTexture)
{
	myCubemap2 = aTexture;
}

void Engine::EnvironmentLightComponent::SetColor(const Vec3f& aColor)
{
	myColor = aColor;
}

void Engine::EnvironmentLightComponent::SetColorIntensity(const float aIntensity)
{
	myColorIntensity = aIntensity;
}

void Engine::EnvironmentLightComponent::SetCubemapIntensity(const float aIntensity)
{
	myCubemapIntensity = aIntensity;
}

const Vec3f& Engine::EnvironmentLightComponent::GetColor() const
{
	return myColor;
}

float Engine::EnvironmentLightComponent::GetColorIntensity() const
{
	return myColorIntensity;
}

float Engine::EnvironmentLightComponent::GetCubemapIntensity() const
{
	return myCubemapIntensity;
}
