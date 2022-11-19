#pragma once

class Player;

namespace Engine
{
	class ModelComponent;
	class VFXComponent;
}

class MagicCircleHandler
{
public:
	void Init(Player* aPlayer);
	void Update();

	GameObject* GetMagicHand();

	Vec3f GetMuzzlePosition();
	Quatf GetMuzzleRotation();
	Vec3f GetMuzzleForward();
	Engine::VFXComponent* GetMuzzleFlash();

	void Primary();

	Vec3f& OffsetRef();
	Vec3f& OffsetSmallRef();
private:
	void UpdatePosition();
	void UpdateVisiblity();
	void UpdatePrimary();
	void EndPrimary();
private:
	Player* myPlayer = nullptr;
	Vec3f myOffset = { 0, 110, 100.f };
	Vec3f mySmallOffset = { 0, 0, 15.f };
	GameObject* myMagicHand = nullptr;
	GameObject* myMainRune = nullptr;
	GameObject* mySmallRune = nullptr;
	Engine::ModelComponent* myMainModel = nullptr;
	Engine::ModelComponent* mySmallModel = nullptr;
	Engine::VFXComponent* myPrimaryMuzzleFlash = nullptr;

	float myVisibilityTarget = 0;
	float myVisiblity = 0;

	float myRotation = 0;
	int myHandBoneIndex = -1;

	struct {
		bool isActive = false;
		float startRotation = 0;
		float targetRotation = 0;

		float timer = 0;
		float time = .5f;

		float recoil = 0;
	} myPrimary;
};

