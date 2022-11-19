#include "pch.h"
#include "EditorCameraMovement.h"
#include "Engine/Engine.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/Renderer/GraphicsEngine.h"

Engine::EditorCameraMovement::EditorCameraMovement(GameObject* aGameObject)
	: Component(aGameObject)
{
}

void Engine::EditorCameraMovement::Execute(eEngineOrder aOrder)
{
	auto& transform = myGameObject->GetTransform();

	auto& input = GetEngine().GetInputManager();

	const auto matrix = transform.GetMatrix();

	bool movedCamera = false;

	if (myIsRightClickDown)
	{
		if (input.IsKeyPressed(C::KeyCode::W))
		{
			auto pos = transform.GetPosition();
			pos += matrix.GetForward() * (mySpeed * Time::DeltaTime);
			transform.SetPosition(pos);
		}

		if (input.IsKeyPressed(C::KeyCode::S))
		{
			auto pos = transform.GetPosition();
			pos -= matrix.GetForward() * (mySpeed * Time::DeltaTime);
			transform.SetPosition(pos);
		}

		if (input.IsKeyPressed(C::KeyCode::A))
		{
			auto pos = transform.GetPosition();
			pos -= matrix.GetRight() * (mySpeed * Time::DeltaTime);
			transform.SetPosition(pos);
		}

		if (input.IsKeyPressed(C::KeyCode::D))
		{
			auto pos = transform.GetPosition();
			pos += matrix.GetRight() * (mySpeed * Time::DeltaTime);
			transform.SetPosition(pos);
		}

		if (input.IsKeyPressed(C::KeyCode::Space))
		{
			auto pos = transform.GetPosition();
			pos += matrix.GetUp() * (mySpeed * Time::DeltaTime);
			transform.SetPosition(pos);
		}

		if (input.IsKeyPressed(C::KeyCode::LeftShift))
		{
			auto pos = transform.GetPosition();
			pos -= matrix.GetUp() * (mySpeed * Time::DeltaTime);
			transform.SetPosition(pos);
		}

		if (input.IsKeyPressed(C::KeyCode::E) && input.IsMouseKeyPressed(C::MouseButton::Right))
		{
			auto pos = transform.GetPosition();
			pos += matrix.GetUp() * (mySpeed * Time::DeltaTime);
			transform.SetPosition(pos);
		}

		if (input.IsKeyPressed(C::KeyCode::Q) && input.IsMouseKeyPressed(C::MouseButton::Right))
		{
			auto pos = transform.GetPosition();
			pos -= matrix.GetUp() * (mySpeed * Time::DeltaTime);
			transform.SetPosition(pos);
		}
	}

	// Prevent the camera from capturing movement
	// when right clicking on anything in imgui
	if (!ImGui::GetIO().WantCaptureMouse || myIsRightClickDown)
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
		}

		if (myIsRightClickDown)
		{
			GetEngine().SetCursorMode(CursorMode::CenterLocked);
			GetEngine().SetCursorVisiblity(false);

			const float cameraSpeedChangeFactor = 100.f;

			// Change the camera speed by scrolling
			mySpeed += static_cast<float>(GetEngine().GetInputManager().GetMouseWheelDelta()) * cameraSpeedChangeFactor;
		}
		else
		{
			GetEngine().SetCursorMode(CursorMode::None);
			GetEngine().SetCursorVisiblity(true);
		}

		if (input.IsMouseKeyPressed(CU::MouseButton::Right))
		{
			constexpr float mouseSensitivity = 0.002f;

			myCurrentRotation.y += mouseDeltaf.x * mouseSensitivity;
			myCurrentRotation.x += mouseDeltaf.y * mouseSensitivity;

			movedCamera = true;
		}
	}

	if (movedCamera)
	{
		Quatf quat;
		quat.InitWithAxisAndRotation(Vec3f(0, 1, 0), myCurrentRotation.y);

		Quatf quat2;
		quat2.InitWithAxisAndRotation(Vec3f(1, 0, 0), myCurrentRotation.x);

		transform.SetRotation(quat * quat2);
	}

	mySpeed = std::max(mySpeed, 1.f);
}

void Engine::EditorCameraMovement::SetSpeed(const float aSpeed)
{
	mySpeed = aSpeed;
}

float Engine::EditorCameraMovement::GetSpeed() const
{
	return mySpeed;
}

void Engine::EditorCameraMovement::SetCurrentRotation(const Vec3f& aCurrentRotation)
{
	myCurrentRotation = aCurrentRotation;
}

const Vec3f& Engine::EditorCameraMovement::GetCurrentRotation() const
{
	return myCurrentRotation;
}

bool Engine::EditorCameraMovement::IsFlying() const
{
	return myIsRightClickDown;
}
