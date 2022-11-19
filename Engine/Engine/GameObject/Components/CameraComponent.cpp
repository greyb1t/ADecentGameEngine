#include "pch.h"
#include "CameraComponent.h"
#include "..\Engine.h"
#include "..\GameObject.h"
#include "Engine/Renderer\Camera\CameraFactory.h"
#include "..\DebugManager\DebugDrawer.h"
#include "Engine/Renderer\Culling\ViewFrustum.h"
#include "Engine/Renderer/Camera/Camera.h"
#include "Engine/DebugManager\DebugMenu.h"
#include "Engine/Renderer/GraphicsEngine.h"

Engine::CameraComponentNew::CameraComponentNew(GameObject* aGameObject)
	: Component(aGameObject)
{
	// auto& cameraFactory = GetEngine().GetCameraFactory();
	// myCamera = cameraFactory.CreatePlayerCamera(60.f);

	// NOTE(filip): when the window size changes, the camera will respond
	auto& windowHandler = GetEngine().GetGraphicsEngine().GetWindowHandler();
	windowHandler.AddObserver(EventType::WindowResized, &myCamera);

	InitCamera();

	// myCamera = &cameraFactory.CreateOrthographicCamera({ 1920, 1080 });

	//auto& transform = myGameObject->GetTransform();
	//auto pos = myGameObject->GetTransform().GetPositionWorld();
	//
	//// myCamera.GetTransform().SetPosition(transform.GetPosition());
	//myCamera.SetPosition(transform.GetPositionWorld());
	////Camera->GetTransform().SetRotationEuler(transform.Rotation);
	////myCamera.GetTransform().SetRotationQuaternion(transform.GetRotation());
	//myCamera.SetRotationQuaternion(transform.GetRotationWorld());
	//
	//// myGameObject->GetScene()->GetRendererScene().SetMainCamera(myCamera);
}

Engine::CameraComponentNew::~CameraComponentNew()
{
	auto& windowHandler = GetEngine().GetGraphicsEngine().GetWindowHandler();
	windowHandler.RemoveObserver(EventType::WindowResized, &myCamera);
}

void Engine::CameraComponentNew::InitCamera()
{
	auto& windowHandler = GetEngine().GetGraphicsEngine().GetWindowHandler();
	myCamera.InitPerspective(myCamera.GetFovDegrees(), windowHandler.GetRenderingSize().CastTo<float>(), myCamera.GetNearPlane(), myCamera.GetFarPlane());
}

void Engine::CameraComponentNew::Execute(eEngineOrder aOrder)
{ 
	// TODO: should be able to override SetPosition()? to avoid doing every frame?
	// or we wont have many cameras, so just do it every frame!

	const auto& transform = myGameObject->GetTransform();

	//myCamera.GetTransform().SetPosition(transform.GetPosition());
	// 
	// MARKUS HAR TAGIT BORT BÖR ANNAN COMPONENT
	myCamera.SetPosition(transform.GetPosition());
	// 
	//myCamera.GetTransform().SetRotationQuaternion(transform.GetRotation());
	// 
	// // MARKUS HAR TAGIT BORT BÖR ANNAN COMPONENT
	myCamera.SetRotationQuaternion(transform.GetRotation());

	//static bool followCam = true;
	//static CU::Vector3f lastPos;

	//if (GetEngine().GetInputManager().IsKeyDown(CU::KeyCode::P))
	//{
	//	followCam = !followCam;
	//}

	//if (followCam)
	//{
	//	lastPos = myCamera.GetTransform().GetPosition();
	//}

	myGameObject->GetScene()->GetRendererScene().GetDebugDrawer().DrawFrustum(
		DebugDrawFlags::Frustum, myCamera.GetViewFrustum().GetLastPos(), myCamera.GetViewFrustum());

	//if (myCamera.GetViewFrustum().GetIsFrozen())
	//{
	//	const auto& resolution = myCamera.GetResolution();
	//	const float ratio = resolution.x / resolution.y;

	//	myCamera.GetViewFrustum().Start(
	//		myFrozenFrustumTransform,
	//		myCamera.GetFovDegrees(),
	//		myCamera.GetNearPlane(),
	//		myCamera.GetFarPlane(),
	//		ratio);

	//	GetEngine().GetDebugDrawer().DrawFrustum(
	//		myFrozenFrustumTransform.GetPosition(),
	//		myCamera.GetViewFrustum());
	//}
	//else
	//{
	//	GetEngine().GetDebugDrawer().DrawFrustum(
	//		myCamera.GetTransform().GetPosition(),
	//		myCamera.GetViewFrustum());
	//}

	if (!GetEngine().IsEditorEnabled())
	{
		if (GetEngine().GetInputManager().IsKeyDown(C::KeyCode::F6))
		{
			static RendererSceneDebugOutput debug = RendererSceneDebugOutput::Albedo;
			myGameObject->GetScene()->GetRendererScene().SetRendererDebugOutput(debug);

			int debugInt = static_cast<int>(debug) + 1;
			if (debugInt >= static_cast<int>(RendererSceneDebugOutput::Count))
			{
				debugInt = 0;
			}
			debug = static_cast<RendererSceneDebugOutput>(debugInt);
		}
	}
}

void Engine::CameraComponentNew::SetFov(float aDegrees)
{
	// auto& cameraFactory = GetEngine().GetCameraFactory();
	// myCamera = &cameraFactory.CreatePlayerCamera(aDegrees);
	myCamera.SetFov(aDegrees);
}

void Engine::CameraComponentNew::Reflect(Reflector& aReflector)
{
	Component::Reflect(aReflector);

	myCamera.Reflect(aReflector, *this);
}

// const Renderer::RTransform& Engine::CameraComponentNew::GetFrozenFrustumTransform() const
// {
// 	return myFrozenFrustumTransform;
// }
//
// void Engine::CameraComponentNew::SetFrozenFrustumTransform(const Renderer::RTransform& aTransform)
// {
// 	myFrozenFrustumTransform = aTransform;
// }
