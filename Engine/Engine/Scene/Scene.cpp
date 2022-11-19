#include "pch.h"
#include "Scene.h"

#include "SceneManager.h"
#include "Engine/Renderer/Scene/RendererScene.h"

#include "Engine/Engine.h"
#include "Engine/DebugManager/DebugDrawer.h"
#include "Engine/Renderer/Camera/Camera.h"
#include "Engine/Renderer/GraphicsEngine.h"
#include "Engine/Renderer/Model/ModelInstance.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/CameraComponent.h"
#include "Engine/GameObject/Components/BasicMovementComponent.h"
#include "Engine/DebugManager/DebugMenu.h"
#include "Engine/Renderer/Culling/ViewFrustum.h"
#include "Engine/ResourceManagement/Resources/TextureResource.h"
#include "Engine/Renderer/Camera/CameraFactory.h"
#include "Engine/GameObject/Components/CameraShakeComponent.h"
#include "Engine/ResourceReferences.h"
#include "Engine/AudioManager.h"
#include "Engine/GameObject/Components/EnvironmentLightComponent.h"
#include "../Renderer/Texture/FullscreenTextureFactory.h"
#include "Engine/Renderer/RenderManager.h"
#include "FolderScene.h"
#include "Engine/Reflection/GameObjectPrefabReferencesReflector.h"

Engine::Scene::Scene()
{
}

Engine::Scene& Engine::Scene::operator=(const Scene& aOther)
{
	Scene scene(aOther);
	std::swap(*this, scene);
	return *this;
}

Engine::Scene::~Scene()
{
	GetEngine().GetGraphicsEngine().GetWindowHandler().RemoveObserver(EventType::WindowResized, this);
}

Engine::Scene::Scene(const Scene& aOther)
	: PhysicsScene(aOther),
	myRenderScene(aOther.myRenderScene),
	myUIRenderScene(aOther.myUIRenderScene),
	myUICamera(aOther.myUICamera),
	myEnvironmentLightFollowCamera(aOther.myEnvironmentLightFollowCamera),
	myResourceReferences(aOther.myResourceReferences),
	myHasBeenInited(aOther.myHasBeenInited),
	myState(aOther.myState.load()),
	myInitState(aOther.myInitState),
	myOnLoadedWorld(aOther.myOnLoadedWorld),
	myIsEditorScene(aOther.myIsEditorScene),
	myName(aOther.myName),
	myIsEnabled(aOther.myIsEnabled),
	// myWindowPosition(aOther.myWindowPosition),
	// myWindowSize(aOther.myWindowSize),
	myClonedScene(aOther.myClonedScene)
{
	assert(myState.is_lock_free());

	//// TODO: In future do Scene::Clone(), GameObjectManager::Clone(Scene&)
	//myGameObjectManager.Init(*this);

	//myGameObjectManager.CloneFromAnother(*this, aOther.myGameObjectManager);

	//myCameraGameObject = FindGameObject("MainCamera");
	//myEnvironmentLightGameObject = FindGameObject("EnvironmentLight");

	//myGameObjectManager.mySingletonComponentEntries.resize(
	//	aOther.myGameObjectManager.mySingletonComponentEntries.size());

	//for (int i = 0; i < aOther.myGameObjectManager.mySingletonComponentEntries.size(); ++i)
	//{
	//	if (aOther.myGameObjectManager.mySingletonComponentEntries[i].myGameObject == nullptr)
	//	{
	//		continue;
	//	}

	//	myGameObjectManager.mySingletonComponentEntries[i].myGameObject =
	//		FindGameObject(aOther.myGameObjectManager.mySingletonComponentEntries[i].myGameObject->GetName());
	//}

	CloneGameObjects(aOther.myGameObjectManager);

	//for (auto g : aOther.myGameObjectManager.mySingletonGameObjects)
	//{
	//	myGameObjectManager.mySingletonGameObjects[g.first] = FindGameObject(g.first);
	//}
}

void Engine::Scene::StartLoading()
{
	// if we are already loaded, don't do it again
	if (myState == State::Ready)
	{
		return;
	}

	// myState = State::NotReady;
	// myInitState = InitState::LoadingResources;

	assert(myHasBeenInited && "init must be called first");

	myResourceReferences.RequestAll();
}

Engine::Scene::Result Engine::Scene::OnRuntimeInit()
{
	//auto cameraGameObject = FindGameObject("MainCamera");

	//if (cameraGameObject == nullptr)
	//{
	//	CreateDefaultCamera();
	//}

	//auto environmentLightGameObject = FindGameObject("EnvironmentLight");

	//if (environmentLightGameObject == nullptr)
	//{
	//	// Create the environment light gameobject
	//	myEnvironmentLightGameObject = AddGameObject<GameObject>();
	//	myEnvironmentLightGameObject->SetName("EnvironmentLight");
	//	myEnvironmentLightGameObject->AddComponent<EnvironmentLightComponent>(
	//		"Assets/EnginePlaceholders/PlaceholderCubemap.dds");
	//}

	return Result::Succeeded;
}

bool Engine::Scene::Init()
{
	GetEngine().GetGraphicsEngine().GetWindowHandler().AddObserver(EventType::WindowResized, this);
	// SetWindowRect({ }, GetEngine().GetWindowSize());

	if (!myGameObjectManager.Init(*this))
	{
		LOG_ERROR(LogType::Engine) << "Failed to init GameObjectManager";
		return false;
	}

	if (!myRenderScene.Init())
	{
		assert(false && "Failed to initialize new renderer scene!");
		return false;
	}

	if (!myUIRenderScene.Init())
	{
		assert(false && "Failed to initialize new UI renderer scene!");
		return false;
	}

	if (!PhysicsScene::Init())
	{
		assert(false && "Failed to init physics scene!");
		return false;
	}

	// if (!CreateDefaultEnvironmentLight("Assets/EnginePlaceholders/PlaceholderCubemap.dds"))
	// {
	// 	return false;
	// }

	// CreateDefaultCamera();
	//  CreateDefaultEnvironmentLight("Assets/EnginePlaceholders/PlaceholderCubemap.dds");

	//// Create the environment light gameobject
	//myEnvironmentLightGameObject = AddGameObject<GameObject>();
	//myEnvironmentLightGameObject->SetName("EnvironmentLight");
	//myEnvironmentLightGameObject->AddComponent<EnvironmentLightComponent>(
	//	"Assets/EnginePlaceholders/PlaceholderCubemap.dds");

	myHasBeenInited = true;

	return true;
}

void Engine::Scene::UpdateInternal(TimeStamp ts)
{
	ZoneScopedN("Scene::UpdateInternal");

	myGameObjectManager.CreateQueuedGameObjects();

	// We never update an editorscene, its supposed to be static
	if (myIsEditorScene)
	{
		return;
	}

	if (!myShouldUpdate)
	{
		return;
	}

	// Must be below if is editor scene, because
	// in editor mode, the make the main camera into the editor camera
	UpdateSingletonGameObjectsRefs();

	switch (myState)
	{
	case State::NotReady:
		UpdateNotFinished(ts);
		break;
	case State::Ready:
		Update(ts);
		break;
	default:
		assert(false);
		break;
	}
}

void Engine::Scene::Update(TimeStamp ts)
{
	AudioManager::GetInstance()->Update();
	ZoneScopedN("Scene::Update");

	// myCameraGameObject = FindGameObject("MainCamera");
	// myEnvironmentLightGameObject = FindGameObject("EnvironmentLight");

	// Hard coded rotation
	//if (myEnvironmentLightGameObject && GetMainCameraGameObject())
	//{
	//	auto& env = myEnvironmentLightGameObject->GetTransform();
	//	EnvironmentLightComponent* envComp = myEnvironmentLightGameObject->GetComponent<EnvironmentLightComponent>();
	//
	//	if (myEnvironmentLightFollowCamera)
	//	{
	//		const auto& camTransform = GetMainCameraGameObject()->GetTransform();
	//		env.SetPosition(camTransform.GetPosition());
	//		env.SetRotation(camTransform.GetRotation());
	//
	//		//myEnvironmentLight.GetCamera()->GetViewFrustum().SetIsFrozen(false);
	//	}
	//	else
	//	{
	//		//myEnvironmentLight.GetCamera()->GetViewFrustum().SetIsFrozen(true);
	//
	//		env.SetPosition(GetMainCameraGameObject()->GetTransform().GetPosition());
	//
	//		const auto rotation = Quatf(0.64f, { 0.149f, 0.733f, -0.17f });
	//		env.SetRotation(rotation);
	//
	//		// We offset the environment light in the opposite of its looking direction
	//		// to fix the issue with shadows being cut off
	//		const auto direction = envComp->GetDirection();
	//		const auto posOffset = -1.f * direction * 10000.f;
	//
	//		// Pos offset to fix the shadow-cutoff
	//		const auto posOffset2 = Vec3f(0.f, 0.f, -500.f);
	//
	//		env.SetPosition(env.GetPosition() + posOffset.ToVec3() + posOffset2);
	//	}
	//
	//	const auto& pos = env.GetPosition();
	//	const auto direction = envComp->GetDirection();
	//
	//	myRenderScene.GetDebugDrawer().DrawSphere3D(DebugDrawFlags::EnvironmentLight, pos, 50.f);
	//
	//	myRenderScene.GetDebugDrawer().DrawLine3D(
	//		DebugDrawFlags::EnvironmentLight, pos, pos + (direction.ToVec3() * 100.f));
	//
	//	/*
	//	GetEngine().GetDebugDrawer().DrawFrustum(
	//		myEnvironmentLight.GetTransform().GetPosition(),
	//		myEnvironmentLight.GetCamera()->GetViewFrustum());
	//	*/
	//}
	//else
	//{
	//	LOG_ERROR(LogType::Engine) << "Missing Camera and/or EnvironmentLight";
	//}

	// auto euler = GetMainCameraGameObject().GetTransform().GetRotation().EulerAngles();

	// myGameObjectManager.CreateQueuedGameObjects();

	myGameObjectManager.Update(ts);
}

void Engine::Scene::LateUpdateInternal(TimeStamp ts)
{
	ZoneScopedN("Scene::LateUpdateInternal");

	// We never update an editorscene, its supposed to be static
	if (myIsEditorScene)
	{
		return;
	}

	if (!myShouldUpdate)
	{
		return;
	}

	switch (myState)
	{
	case State::NotReady:
		break;
	case State::Ready:
		LateUpdate(ts);
		break;
	default:
		assert(false);
		break;
	}
}

void Engine::Scene::LateUpdate(TimeStamp ts)
{
	// We never update an editorscene, its supposed to be static
	if (myIsEditorScene)
	{
		return;
	}

	if (!myShouldUpdate)
	{
		return;
	}

	myRenderScene.LateUpdate(ts);
	myUIRenderScene.LateUpdate(ts);
}

void Engine::Scene::CreateResourceReferences()
{
}

void Engine::Scene::OnSceneSetAsMain()
{
	int test = 0;
}

void Engine::Scene::OnImguiRenderInternal()
{
	ZoneScopedN("Scene::OnImguiRenderInternal");

	switch (myState)
	{
	case State::NotReady:
		break;
	case State::Ready:
		OnImguiRender();
		break;
	default:
		assert(false);
		break;
	}
}

void Engine::Scene::OnImguiRender()
{
}

void Engine::Scene::DrawDebugMenuInternal()
{
	switch (myState)
	{
	case State::NotReady:
		break;
	case State::Ready:
		DrawDebugMenu();
		break;
	default:
		assert(false);
		break;
	}
}

void Engine::Scene::DrawDebugMenu()
{
	std::string menuName = "Scene: " + GetName();

	if (myIsEditorScene)
	{
		menuName += "_EditorScene";
	}

	DebugMenu::AddMenuEntry(
		menuName,
		[this]()
		{
			ImGui::PushID(this);
			{
				if (ImGui::Button("Reload"))
				{
					Reload();
				}
			}

			// Camera
			{
				ImGui::PushID("cam");
				ImGui::Text("Camera");

				auto camera = this->GetMainCameraGameObject();
				const auto& cameraTransform = camera->GetTransform();
				auto camPos = cameraTransform.GetPosition();

				const auto camComponent = camera->GetComponent<CameraComponentNew>();

				ImGui::Text("Position: (%.3f, %.3f, %.3f)", camPos.x, camPos.y, camPos.z);

				ImGui::DragFloat3("Position", &camPos.x);
				camera->GetTransform().SetPosition(camPos);

				auto& rendererCam = camComponent->GetRendererCamera();

				bool frustumFreezed = rendererCam.GetViewFrustum().GetIsFrozen();
				if (ImGui::Checkbox("Freeze Frustum", &frustumFreezed))
				{
					rendererCam.GetViewFrustum().SetIsFrozen(frustumFreezed);
				}

				float fov = rendererCam.GetFovDegrees();
				ImGui::DragFloat("FOV", &fov);
				rendererCam.SetFov(fov);
				ImGui::PopID();
			}

			EnvironmentLightComponent* env = myEnvironmentLightGameObject->GetComponent<EnvironmentLightComponent>();

			// Environment light
			{
				ImGui::PushID("envlight");

				ImGui::Separator();

				ImGui::Text("Environment light");

				std::string cubemap1Path = env->GetCubemap() ? env->GetCubemap()->GetPath() : "";

				ImGui::Text("Cubemap 1: %s", cubemap1Path.c_str());

				std::string cubemap2Path = env->GetCubemap2() ? env->GetCubemap2()->GetPath() : "";

				ImGui::Text("Cubemap 2: %s", cubemap2Path.c_str());

				const auto& envPos = env->GetGameObject()->GetTransform().GetPosition();
				ImGui::Text("Position %.3f, %.3f, %.3f", envPos.x, envPos.y, envPos.z);

				Vec3f color = env->GetColor();
				if (ImGui::ColorEdit3("Color", &color.x))
				{
					env->SetColor(color);
				}

				float colorIntensity = env->GetColorIntensity();

				if (ImGui::DragFloat("Color Intensity", &colorIntensity, 0.1f))
				{
					env->SetColorIntensity(colorIntensity);
				}

				float cubemapIntensity = env->GetCubemapIntensity();

				if (ImGui::DragFloat("Cubemap Intensity", &cubemapIntensity, 0.01f))
				{
					env->SetCubemapIntensity(cubemapIntensity);
				}

				float cubemapInterpolationValue = env->GetCubemapInterpolationValue();

				if (ImGui::DragFloat(
					"Cubemap Interpolation", &cubemapInterpolationValue, 0.01f, -1.f, 1.f))
				{
					env->SetCubemapInterpolationValue(cubemapInterpolationValue);
				}

				ImGui::Checkbox("Follow cam", &myEnvironmentLightFollowCamera);

				ImGui::Separator();

				ImGui::PopID();
			}

			//// Fog
			//{
			//	ImGui::PushID("fog");

			//	ImGui::Text("Fog");

			//	Renderer::FogData fogData = myRenderScene.GetFogData();

			//	if (ImGui::ColorEdit3("Color", &fogData.myColor.x))
			//	{
			//		myRenderScene.SetFogData(fogData);
			//	}

			//	if (ImGui::DragFloat("Fog Start", &fogData.myStart, 10.f))
			//	{
			//		myRenderScene.SetFogData(fogData);
			//	}

			//	if (ImGui::DragFloat("Fog End", &fogData.myEnd, 10.f))
			//	{
			//		myRenderScene.SetFogData(fogData);
			//	}

			//	if (ImGui::DragFloat("Fog Strength", &fogData.myFogStrength, 0.01f, 0.f, 1.f))
			//	{
			//		myRenderScene.SetFogData(fogData);
			//	}

			//	ImGui::Separator();

			//	ImGui::PopID();
			//}

			// Shadows
			{
				ImGui::PushID("shadow");

				float shadowStrength = myRenderScene.GetShadowStrength();

				if (ImGui::DragFloat("Shadow Strength", &shadowStrength, 0.01f, 0.f, 1.f))
				{
					myRenderScene.SetShadowStrength(shadowStrength);
				}

				ImGui::PopID();
			}

			ImGui::PopID();
		},
		this);
}

void Engine::Scene::UpdateSystems(eEngineOrder aOrder)
{
	// We never update an editorscene, its supposed to be static
	if (myIsEditorScene)
	{
		return;
	}

	if (!myShouldUpdate)
	{
		return;
	}

	switch (myState)
	{
	case State::NotReady:
		break;
	case State::Ready:
		myGameObjectManager.UpdateSystems(aOrder);
		break;
	default:
		assert(false);
		break;
	}

	// NOTE(filip): Check if new gameobjects were created, if so update them
	// why? because since physics run right after doing a regular UPDATE
	// on the gameobjects, the next step runs physics, therefore,
	// physics does stuff on gameobjects that has not yet been called Start()
	// So jesper had the issue where he created GameObjects, and in a component Start()
	// he set the gameobject Layers, but that was not run yet and physics ran before that
	// causing it to the first frame have incorrect values and causing it to collide
	myGameObjectManager.CreateQueuedGameObjects();
}

void Engine::Scene::Render()
{
	ZoneScopedN("Scene::Render");

	switch (myState)
	{
	case State::NotReady:
		break;

	case State::Ready:
	{
		// Swap the render target textures
		// the reason we have 2 of them it because
		// we need to render stuff to ImGui windows from the textures
		// therefore we need the previous frames result to avoid calling
		// directx Present() multiple times and shit like that
		myRenderScene.SwapRenderTargets();

		// Must be called here, otherwise it will crash when
		// changing scene with the debug menu visible
		DrawDebugMenu();

		myGameObjectManager.Render();

		if (myCameraGameObject && myEnvironmentLightGameObject)
		{
			auto& cam
				= myCameraGameObject->GetComponent<CameraComponentNew>()->GetRendererCamera();
			EnvironmentLightComponent* env = myEnvironmentLightGameObject->GetComponent<EnvironmentLightComponent>();

			myRenderScene.PrepareRendering(cam, *env);
			myUIRenderScene.PrepareRendering(myUICamera, *env);
		}
	}
	break;

	default:
		assert(false);
		break;
	}
}

void Engine::Scene::BeginRenderReal(GraphicsEngine& aGraphicsEngine)
{
	ZoneScopedN("Scene::BeginRenderReal");

	const float deltaTime = GetEngine().GetTimer().GetDeltaTime();

	// Submit the render calls to the DebugRenderer
	myRenderScene.GetDebugDrawer().Update(
		deltaTime, aGraphicsEngine.GetRenderManager().GetDebugRenderer());

	aGraphicsEngine.RenderFrame(myRenderScene, &myUIRenderScene);
}

void Engine::Scene::EndRenderReal(GraphicsEngine& aGraphicsEngine)
{
	ZoneScopedN("Scene::EndRenderReal");

	aGraphicsEngine.EndRenderFrame(myRenderScene);
}

const std::vector<Shared<GameObject>>& Engine::Scene::GetGameObjects() const
{
	return myGameObjectManager.GetGameObjects();
}

GameObject* Engine::Scene::FindGameObject(int aUUID)
{
	return myGameObjectManager.FindGameObject(aUUID);
}

GameObject* Engine::Scene::FindGameObject(const std::string& aName) const
{
	return myGameObjectManager.FindGameObject(aName);
}

std::vector<GameObject*> Engine::Scene::FindGameObjects(eTag)
{
	return std::vector<GameObject*>();
}

int Engine::Scene::CountGameObjectsWithName(const std::string& aName) const
{
	return std::count_if(myGameObjectManager.myGameObjects.begin(), myGameObjectManager.myGameObjects.end(),
		[aName](const Shared<GameObject>& aGameObject)
		{
			return aGameObject->GetName() == aName;
		});
}

bool Engine::Scene::ScreenToWorld(
	const C::Vector2<unsigned int>& aScreenPosition, unsigned int aLayerMask, Vec3f& outPosition)
{
	Vec3f rayDir = ScreenToWorldDirection(aScreenPosition);

	std::vector<RayCastHit> hits;
	const float distance = 10000;
	auto cam = myRenderScene.GetMainCamera();
	const Vec3f& camPos = cam->GetTransform().GetPosition();
	if (RayCastAll(camPos, rayDir, distance, aLayerMask, hits, false))
	{
		RayCastHitInternal hit = hits[0];
		for (int i = 1; i < hits.size(); i++)
		{
			if (hits[i].Distance < hit.Distance)
			{
				hit = hits[i];
			}
		}

		outPosition = hit.Position;
		return true;
	}
	return false;
}

Vec3f Engine::Scene::ScreenToWorldDirection(const Vec2ui& aScreenPosition)
{
	Vec2f normScreenPos;

	normScreenPos.x = 2.f
		* (aScreenPosition.x
			/ static_cast<float>(Engine::Engine::GetInstance().GetRenderingSize().x - 1))
		- 1.f;
	normScreenPos.y = 1.f
		- 2.f
		* (aScreenPosition.y
			/ static_cast<float>(Engine::Engine::GetInstance().GetRenderingSize().y - 1));

	auto cam = myRenderScene.GetMainCamera();

	if (!cam)
	{
		return { 0, 0, 1 };
	}
	Mat4f inverseProjection = cam->GetProjectionMatrix().Inverse();

	Mat4f multiplied = inverseProjection * cam->GetTransform().ToMatrix();

	C::Vector4f nearVector;
	nearVector.x = normScreenPos.x;
	nearVector.y = normScreenPos.y;

	// NOTE(Filip): I removed GetNearPlane(), because when I changed
	// nearplane to 50, ScreenToWorldDirection(), it stopped working
	// however, the Z value below is the depth, and we can manually set it somewhere between -1 and 1
	nearVector.z = /*cam->GetNearPlane()*/ 1.f;

	nearVector.w = 1.f;
	nearVector = nearVector * multiplied;

	if (nearVector.w != 0)
	{
		nearVector.x /= nearVector.w;
		nearVector.y /= nearVector.w;
		nearVector.z /= nearVector.w;
	}

	const Vec3f& camPos = cam->GetTransform().GetPosition();
	Vec3f rayDir = { nearVector.x - camPos.x, nearVector.y - camPos.y, nearVector.z - camPos.z };
	rayDir.Normalize();

	return rayDir;
}

bool Engine::Scene::RayCast(const Vec3f& aOrigin, const Vec3f& aDirection, float aRange, LayerMask aLayerMask, RayCastHit& outHit, bool aFindGameObject)
{
#ifndef _RETAIL
	float length = aDirection.LengthSqr();
	if (length < 0.999f || length > 1.001)
	{
		return false;
	}
#endif
	if (PhysicsScene::RayCast(aOrigin, aDirection, aRange, aLayerMask, outHit))
	{
		if (outHit.Actor)
		{
			if (auto userData = outHit.Actor->userData)
			{
				outHit.UUID = *static_cast<int*>(userData);
				if (aFindGameObject && outHit.UUID != -1)
				{
					outHit.GameObject = FindGameObject(outHit.UUID);
				}
			}
		}
		return true;
	}
	return false;
}

bool Engine::Scene::RayCastAll(const Vec3f& aOrigin, const Vec3f& aDirection, float aRange, LayerMask aLayerMask, std::vector<RayCastHit>& outHit, bool aFindGameObject)
{
#ifndef _RETAIL
	float length = aDirection.LengthSqr();
	if (length < 0.999f || length > 1.001)
	{
		return false;
	}
#endif
	std::vector<RayCastHitInternal> internalHits;
	if (PhysicsScene::RayCastAll(aOrigin, aDirection, aRange, aLayerMask, internalHits))
	{
		for (auto& hit : internalHits)
		{
			if (hit.Actor)
			{
				if (auto userData = hit.Actor->userData)
				{
					RayCastHit raycastHit(hit);

					raycastHit.UUID = *static_cast<int*>(userData);
					if (aFindGameObject && raycastHit.UUID != -1)
					{
						raycastHit.GameObject = FindGameObject(raycastHit.UUID);
					}

					outHit.emplace_back(raycastHit);
				}
			}
		}
		return true;
	}
	return false;
}

bool Engine::Scene::BoxCastAll(const Vec3f& aPosition, const Quatf& aQuaternion, const Vec3f& aHalfSize, LayerMask aLayerMask, std::vector<OverlapHit>& outHits, bool aFindGameObject)
{
	std::vector<OverlapHitInternal> internalHits;
	if (PhysicsScene::BoxCastAll(aPosition, aQuaternion, aHalfSize, aLayerMask, internalHits))
	{
		for (auto& hit : internalHits)
		{
			if (hit.Actor)
			{
				if (auto userData = hit.Actor->userData)
				{
					OverlapHit overlapHit(hit);

					overlapHit.UUID = *static_cast<int*>(userData);
					if (aFindGameObject && overlapHit.UUID != -1)
					{
						overlapHit.GameObject = FindGameObject(overlapHit.UUID);
					}

					outHits.emplace_back(overlapHit);
				}
			}
		}
		return true;
	}
	return false;
}

bool Engine::Scene::SphereCastAll(const Vec3f& aPosition, float aRadius, LayerMask aLayerMask, std::vector<OverlapHit>& outHits, bool aFindGameObject)
{
	std::vector<OverlapHitInternal> internalHits;
	if (PhysicsScene::SphereCastAll(aPosition, aRadius, aLayerMask, internalHits))
	{
		for (auto& hit : internalHits)
		{
			if (hit.Actor)
			{
				if (auto userData = hit.Actor->userData)
				{
					OverlapHit overlapHit(hit);

					overlapHit.UUID = *static_cast<int*>(userData);
					if (aFindGameObject && overlapHit.UUID != -1)
					{
						overlapHit.GameObject = FindGameObject(overlapHit.UUID);
					}

					outHits.emplace_back(overlapHit);
				}
			}
		}
		return true;
	}
	return false;
}

bool Engine::Scene::GeometryCastAll(const physx::PxGeometry& aGeometry, const Vec3f& aPosition, const Quatf& aQuat, LayerMask aLayerMask, std::vector<OverlapHit>& outHits, bool aFindGameObjects /*= true*/)
{
	std::vector<OverlapHitInternal> internalHits;
	if (PhysicsScene::GeometryCastAll(aGeometry, aPosition, aQuat, aLayerMask, internalHits))
	{
		for (auto& hit : internalHits)
		{
			if (hit.Actor)
			{
				if (auto userData = hit.Actor->userData)
				{
					OverlapHit overlapHit(hit);
					overlapHit.UUID = *static_cast<int*>(userData);
					if (aFindGameObjects && overlapHit.UUID != -1)
					{
						overlapHit.GameObject = FindGameObject(overlapHit.UUID);
					}

					outHits.emplace_back(overlapHit);
				}
			}
		}
		return true;
	}
	return false;
}

GameObject* Engine::Scene::GetMainCameraGameObject()
{
	return myCameraGameObject;
}

Engine::Camera& Engine::Scene::GetMainCamera()
{
	return myCameraGameObject->GetComponent<CameraComponentNew>()->GetRendererCamera();
}

void Engine::Scene::SetName(const std::string& aName)
{
	myName = aName;
}

Engine::RendererScene& Engine::Scene::GetRendererScene()
{
	return myRenderScene;
}

Engine::RendererScene& Engine::Scene::GetUIRendererScene()
{
	return myUIRenderScene;
}

Engine::ComponentSystem& Engine::Scene::GetComponentSystem()
{
	return myGameObjectManager.GetComponentSystem();
}

bool Engine::Scene::HasBeenInited() const
{
	return myHasBeenInited;
}

bool Engine::Scene::CreateDefaultEnvironmentLight()
{
	myEnvironmentLightGameObject = AddGameObject<GameObject>();
	myEnvironmentLightGameObject->SetName("EnvironmentLight");
	myEnvironmentLightGameObject->AddComponent<EnvironmentLightComponent>(
		"Assets/Engine/PlaceholderTextures/PlaceholderCubemap.dds");

	// myEnvironmentLightGameObject->GetComponent<EnvironmentLightComponent>()->CreateIt(aCubemapPath);

	return true;
}

bool Engine::Scene::IsReady() const
{
	return myState == State::Ready;
}

void Engine::Scene::SetOnLoadedWorldCallback(std::function<void()> aOnLoadedWorld)
{
	myOnLoadedWorld = aOnLoadedWorld;
}

void Engine::Scene::SetIsEditorScene(const bool aIsEditorScene)
{
	myIsEditorScene = aIsEditorScene;
}

bool Engine::Scene::IsEditorScene() const
{
	return myIsEditorScene;
}

void Engine::Scene::SetShouldUpdate(const bool aShouldUpdate)
{
	myShouldUpdate = aShouldUpdate;
}

bool Engine::Scene::ShouldUpdate() const
{
	return myShouldUpdate;
}

Shared<GameObject> Engine::Scene::CreateGameObject()
{
	auto g = myGameObjectManager.CreateGameObject();
	g->myScene = this;

	return g;
}

void Engine::Scene::SetMainCamera(GameObject* aGameObject)
{
	myCameraGameObject = aGameObject;
}

Engine::Scene::InitState Engine::Scene::GetInitState() const
{
	return myInitState;
}

void Engine::Scene::UpdateNotFinished(TimeStamp ts)
{
	ZoneScopedN("Scene::UpdateNotFinished");

	switch (myInitState)
	{
	case InitState::LoadResources:
	{
		// if (myResourceReferences->AreAllFinishedLoading())
		// NOTE(Filip): Problem is that we are waiting for all models
		// but the models themselves load textures that are not in
		// the resourcerefs so we cannot wait for them
		// therefore we wait for all resources atm
		// TODO: Fix so a resource can have references to another
		// in the meta data?
		// EDIT: To solve that issue, we can do one extra iteration
		// in other words, use 1 extra frame to ensure no other resource
		// loaded another resource

		/*

			// All resources within this group, are stored together and used to determine if
			// FUCK, won't work because we are not in a group if we CreateRef
			// within CreateFbx()

			ResourceManager::BeginGroup()
			...
			ResourceManager::EndGroup()

		 */

		 //if (GetEngine().GetResourceManager().IsAllResourcesLoaded())
		if (myResourceReferences.AreAllFinishedLoading()
			// NOTE(filip): due to loading resource in other resources
			// i have to do this check as well, otherwise textures that were
			// loaded from models, won't be waited for..FUCK
			/*&& GetEngine().GetResourceManager().IsAllResourcesLoaded()*/)
		{
			// Frame wait counter
			// A resource can load another resource from its Create()
			// e.g. a model can load textures
			// to support that, we must
			// nvm, not possible, we never add the child resources into this resource refs...

			myInitState = InitState::LoadScene;
			myLoadingPercent =
				static_cast<float>(static_cast<int>(myInitState)) /
				static_cast<float>(static_cast<int>(InitState::Count));
		}
		else
		{
			const float bigPicturePercent = 
				static_cast<float>(static_cast<int>(myInitState)) /
				static_cast<float>(static_cast<int>(InitState::Count));
			myLoadingPercent = myResourceReferences.GetPercentageLoaded() * bigPicturePercent;
		}
	}
	break;

	case InitState::LoadScene:
	{
		const Result state = LoadScene();

		if (state == Result::Succeeded)
		{
			myInitState = InitState::LoadWorld;

			myLoadingPercent =
				static_cast<float>(static_cast<int>(myInitState)) /
				static_cast<float>(static_cast<int>(InitState::Count));
		}
		else if (state == Result::Failed)
		{
			myState = State::Failed;
		}
	}
	break;

	case InitState::LoadWorld:
	{
		const Result state = LoadWorld();

		if (state == Result::Succeeded)
		{
			myInitState = InitState::RuntimeInit;

			myLoadingPercent =
				static_cast<float>(static_cast<int>(myInitState)) /
				static_cast<float>(static_cast<int>(InitState::Count));

			// Clone the scene and keep a unchanged version of it
			// to go back to an unchanged version
			// this is just to avoid reloading the scene from disk
			// when dying with the player
			{
				if (myClonedScene == nullptr)
				{
					auto clonedScene = Clone();

					if (clonedScene != nullptr)
					{
						FolderScene* clonedFolderScene = dynamic_cast<FolderScene*>(clonedScene);

						if (clonedFolderScene)
						{
							myClonedScene = std::shared_ptr<FolderScene>(clonedFolderScene);
						}
					}
				}
			}

			if (myOnLoadedWorld)
			{
				myOnLoadedWorld();
			}
		}
		else if (state == Result::Failed)
		{
			myState = State::Failed;
		}
	}
	break;

	case InitState::RuntimeInit:
	{
		// We want to support returning false while still initializting,
		// the reason is so we can possibly thread the loading of the world and such...
		const Result state = OnRuntimeInit();

		if (state == Result::Succeeded)
		{
			for (auto& g : myGameObjectManager.myGameObjects)
			{
				GameObjectPrefabReferencesReflector refl(myGameObjectManager, *g);

				g->Reflect(refl);

				for (auto& c : g->myComponents)
				{
					c->Reflect(refl);
				}
			}

			for (auto& g : myGameObjectManager.myGameObjectsToAdd)
			{
				GameObjectPrefabReferencesReflector refl(myGameObjectManager, *g);

				g->Reflect(refl);

				for (auto& c : g->myComponents)
				{
					c->Reflect(refl);
				}
			}

			/*
			// Update the reflected references e.g. gameobject references
			GameObjectPrefabReferencesReflector refl(myGameObjectManager);
			myGameObjectManager.ReflectGameObjects(refl);
			myGameObjectManager.ReflectAllGameObjectComponents(refl);
			*/

			myInitState = InitState::None;

			myLoadingPercent = 1.f;

			myState = State::Ready;
		}
		else if (state == Result::Failed)
		{
			myState = State::Failed;
		}
	}
	break;

	default:
		break;
	}
}

void Engine::Scene::UpdateSingletonGameObjectsRefs()
{
	ZoneScopedN("Scene::UpdateSingletonGameObjectsRefs");

	// A singleton gameobject is only allowed to have one instance of that name
	myCameraGameObject = FindGameObject("MainCamera");
	myEnvironmentLightGameObject = FindGameObject("EnvironmentLight");

	// Only update the singleton component values if we never found it before
	for (const auto& singletonComponent : SingletonComponentFactory::GetInstance().GetSingletonComponents())
	{
		// Ensure big enough
		if (singletonComponent.myID >= myGameObjectManager.mySingletonComponentEntries.size())
		{
			myGameObjectManager.mySingletonComponentEntries.resize(singletonComponent.myID + 1);
		}

		auto& entry = myGameObjectManager.mySingletonComponentEntries[singletonComponent.myID];

		if (entry.myComponent == nullptr)
		{
			GameObject* g = FindGameObject(singletonComponent.myGameObjectName);

			if (g == nullptr)
			{
				continue;
			}

			if (CountGameObjectsWithName(singletonComponent.myGameObjectName) > 1)
			{
				assert(false && "not allowed to have multiple singleton gameobjects");

				LOG_ERROR(LogType::Engine) << "You have multiple singleton gameobject in the scene: " << singletonComponent.myGameObjectName;
			}

			entry.myGameObject = g;

			entry.myComponent = entry.myGameObject->GetComponentByTypeId(singletonComponent.myComponentID);
		}
	}
}

void Engine::Scene::CloneGameObjects(const GameObjectManager& aGameObjectManager)
{
	// TODO: In future do Scene::Clone(), GameObjectManager::Clone(Scene&)
	myGameObjectManager.Init(*this);

	myGameObjectManager.CloneFromAnother(*this, aGameObjectManager);

	myCameraGameObject = FindGameObject("MainCamera");
	myEnvironmentLightGameObject = FindGameObject("EnvironmentLight");

	myGameObjectManager.mySingletonComponentEntries.resize(
		aGameObjectManager.mySingletonComponentEntries.size());

	for (int i = 0; i < aGameObjectManager.mySingletonComponentEntries.size(); ++i)
	{
		if (aGameObjectManager.mySingletonComponentEntries[i].myGameObject == nullptr)
		{
			continue;
		}

		myGameObjectManager.mySingletonComponentEntries[i].myGameObject =
			FindGameObject(aGameObjectManager.mySingletonComponentEntries[i].myGameObject->GetName());
	}
}

void Engine::Scene::Receive(const EventType aEventType, const std::any& aValue)
{
	const auto& renderSizef = GetEngine().GetGraphicsEngine().GetWindowHandler().GetRenderingSize<float>();

	// myWindowSize = renderSizef;
}

bool Engine::Scene::IsEnabled() const
{
	return myIsEnabled;
}

void Engine::Scene::SetIsEnabled(const bool aIsEnabled)
{
	myIsEnabled = aIsEnabled;
}

bool Engine::Scene::IsSceneBeingEdited() const
{
	return mySceneBeingEdited;
}

GameObject* Engine::Scene::GetEnvironmentLight()
{
	return myEnvironmentLightGameObject;
}

void Engine::Scene::Reload()
{
	Time::PauseTimeScale = 1;

	if (myClonedScene == nullptr)
	{
		LOG_ERROR(LogType::Engine) << "Unable to reload scene because no clone exists";
		return;
	}

	auto clone = myClonedScene->Clone();

	// Remove the old gameobject
	myGameObjectManager = { };

	CloneGameObjects(clone->myGameObjectManager);

	delete clone;

	OnRuntimeInit();
}

float Engine::Scene::GetLoadingPercent() const
{
	return myLoadingPercent;
}

void Engine::Scene::CreateDefaultCamera()
{
	assert(myCameraGameObject == nullptr);

	// Create the main camera
	myCameraGameObject = AddGameObject<GameObject>();
	myCameraGameObject->SetName("MainCamera");
	auto cameraComponentNew = myCameraGameObject->AddComponent<CameraComponentNew>();
	myCameraGameObject->AddComponent<BasicMovementComponentNew>();
	myCameraGameObject->AddComponent<CameraShakeComponent>();

	myUICamera = GetEngine().GetCameraFactory().CreateUICamera();

	// SetMainCamera(myCameraGameObject);
}

Engine::ResourceReferences& Engine::Scene::GetResourceReferences()
{
	return myResourceReferences;
}
