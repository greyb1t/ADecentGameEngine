#include "pch.h"
#include "BasicMovementComponent.h"
#include "..\GameObject.h"
#include "..\Engine.h"
#include "Engine/Renderer/GraphicsEngine.h"

Engine::BasicMovementComponentNew::BasicMovementComponentNew(GameObject* aGameObject)
	: Component(aGameObject)
{
	int test = 0;
}

void Engine::BasicMovementComponentNew::Execute(eEngineOrder aOrder)
{
#ifndef _RETAIL
	if (GetEngine().GetInputManager().IsKeyDown(C::KeyCode::F3))
	{
		myIsMoveActive = !myIsMoveActive;
	}
#endif

	if (myIsMoveActive == false)
		return;


	auto& transform = myGameObject->GetTransform();

	auto& input = GetEngine().GetInputManager();

	const auto matrix = transform.GetMatrix();

	bool movedCamera = false;

	float speed = Speed;

	if (myIsRightClickDown)
	{
		// Remove because we have keybind Ctrl+S to save in editor and this collides with that
		//if (input.IsKeyPressed(C::KeyCode::LeftControl))
		//{
		//	speed *= 5.f;
		//}

		if (input.IsKeyPressed(C::KeyCode::W))
		{
			auto pos = transform.GetPosition();
			pos += matrix.GetForward() * (speed * Time::DeltaTime);
			transform.SetPosition(pos);
		}

		if (input.IsKeyPressed(C::KeyCode::S))
		{
			auto pos = transform.GetPosition();
			pos -= matrix.GetForward() * (speed * Time::DeltaTime);
			transform.SetPosition(pos);
		}

		if (input.IsKeyPressed(C::KeyCode::A))
		{
			auto pos = transform.GetPosition();
			pos -= matrix.GetRight() * (speed * Time::DeltaTime);
			transform.SetPosition(pos);
		}

		if (input.IsKeyPressed(C::KeyCode::D))
		{
			auto pos = transform.GetPosition();
			pos += matrix.GetRight() * (speed * Time::DeltaTime);
			transform.SetPosition(pos);
		}

		if (input.IsKeyPressed(C::KeyCode::Space))
		{
			auto pos = transform.GetPosition();
			pos += matrix.GetUp() * (speed * Time::DeltaTime);
			transform.SetPosition(pos);
		}

		if (input.IsKeyPressed(C::KeyCode::LeftShift))
		{
			auto pos = transform.GetPosition();
			pos -= matrix.GetUp() * (speed * Time::DeltaTime);
			transform.SetPosition(pos);
		}
	}

	const float rotationSpeed = 0.9f;

	//if (input.IsKeyPressed(C::KeyCode::Left))
	//{
	//	CurrentRotationEuler.y -= rotationSpeed * Time::DeltaTime;

	//	movedCamera = true;
	//}

	//if (input.IsKeyPressed(C::KeyCode::LocalRight))
	//{
	//	CurrentRotationEuler.y += rotationSpeed * Time::DeltaTime;
	//	movedCamera = true;
	//}

	//if (input.IsKeyPressed(C::KeyCode::LocalUp))
	//{
	//	CurrentRotationEuler.x -= rotationSpeed * Time::DeltaTime;
	//	movedCamera = true;
	//}

	//if (input.IsKeyPressed(C::KeyCode::Down))
	//{
	//	CurrentRotationEuler.x += rotationSpeed * Time::DeltaTime;
	//	movedCamera = true;
	//}

	// Prevent the camera from capturing movement
	// when right clicking on anything in imgui
	if (!ImGui::GetIO().WantCaptureMouse || myIsRightClickDown) //Simon - Kommenterar bort detta för att de releaser musen när man right clickar ingame
	{
		const auto mouseDeltaPoint = input.GetMouseDeltaRaw();
		const auto mouseDeltaf = Vec2f(
			static_cast<float>(mouseDeltaPoint.x),
			static_cast<float>(mouseDeltaPoint.y));

		if (input.IsMouseKeyDown(CU::MouseButton::Right))
		{
			//ShowCursor(FALSE);
			input.HideCursorGuaranteed();
			myIsRightClickDown = true;
		}

		if (input.WasMouseKeyReleased(CU::MouseButton::Right))
		{
			// ugly, but needed because I can't bother to find a better solution atm
			// since ShowCursor() increases the ref count, there may be an unbalance
			// therefore, show call it until its done

			// ShowCursor(TRUE);
			input.ShowCursorGuaranteed();

			myIsRightClickDown = false;

			GetEngine().SetCursorMode(CursorMode::None);
			GetEngine().SetCursorVisiblity(true);
		}

		if (myIsRightClickDown)
		{
			GetEngine().SetCursorMode(CursorMode::CenterLocked);
			GetEngine().SetCursorVisiblity(false);
		}

		if (input.IsMouseKeyPressed(CU::MouseButton::Right))
		{
			constexpr float mouseSensitivity = 0.002f;

			CurrentRotationEuler.y += mouseDeltaf.x * mouseSensitivity;
			CurrentRotationEuler.x += mouseDeltaf.y * mouseSensitivity;

			movedCamera = true;
		}
	}
	if (movedCamera)
	{
		Quatf quat;
		quat.InitWithAxisAndRotation(Vec3f(0, 1, 0), CurrentRotationEuler.y);

		Quatf quat2;
		quat2.InitWithAxisAndRotation(Vec3f(1, 0, 0), CurrentRotationEuler.x);

		//transform.RotationQuat = quat * quat2;
		transform.SetRotation(quat * quat2);
	}


	//transform.RotationQuat = CU::Quaternion(movement.CurrentRotationEuler);
}

void Engine::BasicMovementComponentNew::IsMoveActive(bool aState)
{
	myIsMoveActive = aState;
}

bool Engine::BasicMovementComponentNew::GetIsMoveActive() const
{
	return myIsMoveActive;
}

void Engine::BasicMovementComponentNew::SetSpeed(const float aSpeed)
{
	Speed = aSpeed;
}

float Engine::BasicMovementComponentNew::GetSpeed() const
{
	return Speed;
}