#include "pch.h"
#include "DamageNumber.h"

#include "Engine/Shortcuts.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/TextComponent.h"
#include "Game/Components/HealthComponent.h"

DamageNumber::DamageNumber(GameObject* aObj, int aNumber)
	: Component(aObj), myNumber(aNumber)
{
}

void DamageNumber::Start()
{
	int number = std::roundf(CU::Max(myNumber, 1));
	myText = myGameObject->AddComponent<Engine::TextComponent>(std::to_string(number), 0);
	myText->SetColor(myTextColor);
	myGameObject->SetName("Damage Number");
	GetTransform().SetScale({ .5f, .5f, .5f });

	const Vec3f dir = myGameObject->GetScene()->GetMainCamera().GetTransform().GetPosition() - GetTransform().GetPosition();
	const Engine::RTransform& cameraTransform = myGameObject->GetScene()->GetMainCamera().GetTransform();
	Mat4f cameraMatrix = cameraTransform.ToMatrix();
	GetTransform().Move(cameraMatrix.GetRight() * dir.Length() / 400.f * Random::RandomFloat(-10.f, 10.f));

	SetTextScale(myScaleModifier);

	myOriginalScale = myText->GetTransform().GetScale();
}

void DamageNumber::SetColor(const CU::Vector4f& aColor)
{
	myTextColor = aColor;
	if (myText)
	{
		myText->SetColor(aColor);
	}
}

void DamageNumber::SetTextScale(const float aScale)
{
	myScaleModifier = aScale;
	if (myText)
	{
		const Vec3f dir = myGameObject->GetScene()->GetMainCamera().GetTransform().GetPosition() - GetTransform().GetPosition();
		myText->GetTransform().SetScale((dir.Length() / 2000) * myScaleModifier);
	}
}

void DamageNumber::SetPreset(eDamageType aPreset)
{
	switch (aPreset)
	{
	case eDamageType::Basic:
		SetColor({ .8f, .8f, 0.8f, 1.f });
		SetTextScale(1.f);

		break;
	case eDamageType::Crit:
		SetColor({ 1.f, 0.f, 0.f, 1.f });
		SetTextScale(1.2f);
		myShakeOffset = { Random::RandomFloat(-10.f, 10.f), Random::RandomFloat(-10.f, 10.f) };
		myShakeOffset = myShakeOffset.GetNormalized() * 45.f;
		myShakeInterval = 0.05f;
		myShakeLengthReduction = 0.85f;
		break;
	case eDamageType::Bleed:
		SetColor({ 1.f, 0.f, 0.f, 1.f });
		myRiseSpeed = 400.f;
		break;
	case eDamageType::Poison:
		SetColor({ 0.769f, 0.153, 0.482f, 1.f });
		break;
	default:
		LOG_ERROR(LogType::Game) << "Unsupported damage number preset!";
	}

	const Engine::RTransform& cameraTransform = myGameObject->GetScene()->GetMainCamera().GetTransform();
	Mat4f cameraMatrix = cameraTransform.ToMatrix();
	GetTransform().Move(cameraMatrix.GetRight() * myShakeOffset.x);
	GetTransform().Move(cameraMatrix.GetUp() * myShakeOffset.y);
}

void DamageNumber::Execute(Engine::eEngineOrder aOrder)
{
	myTime += Time::DeltaTime;
	if (myTime >= myLifetime)
	{
		myGameObject->Destroy();
		return;
	}

	bool shake = false;
	if (myShakeInterval > 0.f)
	{
		myShakeTimer -= Time::DeltaTime;
		if (myShakeTimer < 0.f)
		{
			myShakeTimer = myShakeInterval;
			shake = true;
		}
	}

	const float percentage = 1.f - (myTime / myLifetime);
	Vec2f shakeOffset = { 0.f, 0.f };
	if (shake)
	{
		shakeOffset -= myShakeOffset;
		float shakeLength = myShakeOffset.Length();
		myShakeOffset.x = Random::RandomFloat(-10.f, 10.f);
		myShakeOffset.y = Random::RandomFloat(-10.f, 10.f);
		myShakeOffset = myShakeOffset.GetNormalized() * shakeLength * myShakeLengthReduction;
		shakeOffset += myShakeOffset;
	}

	float dist = myRiseSpeed * Time::DeltaTime * CU::Max(powf(percentage, 2), 0.f);
	GetTransform().Move(Vec3f(0, dist, 0));
	const Engine::RTransform& cameraTransform = myGameObject->GetScene()->GetMainCamera().GetTransform();
	Mat4f cameraMatrix = cameraTransform.ToMatrix();
	GetTransform().Move(cameraMatrix.GetRight() * shakeOffset.x);
	GetTransform().Move(cameraMatrix.GetUp() * shakeOffset.y);

	const Vec3f dir = cameraTransform.GetPosition() - GetTransform().GetPosition();
	GetTransform().SetRotation(Quatf(Vec3f(0, (Math::AngleOfVector({ dir.x, dir.z }) + 90.f) * Math::Deg2Rad, 0)));

	myText->GetTransform().SetScale(Math::Lerp(myOriginalScale * 0.5f, myOriginalScale, percentage));
}

