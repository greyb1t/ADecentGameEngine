#include "pch.h"
#include "Explosion.h"
#include "Engine/ResourceManagement/Resources/AnimationCurveResource.h"
#include "Engine/AnimationCurve/Curve.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/Scene/Scene.h"
#include <Components/HealthComponent.h>
#include <Player/Player.h>
#include <AI/PollingStation/PollingStationComponent.h>
#include "Engine/GameObject/Components/ModelComponent.h"

Explosion::Explosion(GameObject* aGameObject)
	: Component(aGameObject)
{

}

Explosion::~Explosion()
{
}

void Explosion::Init(ExplosionInfo aExplosion, DamageInfo aDamage)
{
	myExplosion = aExplosion;
	myDamage = aDamage;

	myInitialSize = GetGameObject()->GetTransform().GetScale() / 100.f;

	myModel = myGameObject->GetComponent<Engine::ModelComponent>();

}

void Explosion::Execute(Engine::eEngineOrder aOrder)
{
	UpdateExplosion();
}

void Explosion::UpdateExplosion()
{
	myTimer += Time::DeltaTime;
	float percentage = myTimer / myExplosion.explosionTime;

	if (percentage > 1.f)
	{
		percentage = 1.f;
		myGameObject->Destroy();
	}

	float size = myExplosion.range;

	if (myExplosion.sizeCurve && myExplosion.sizeCurve->IsValid())
	{
	 	size *= myExplosion.sizeCurve->Get().Evaluate(percentage);
	}

	GetTransform().SetScale(myInitialSize * size);

	ScanDamage(size);
	if (myModel) {
		auto& matInst = myModel->GetMeshMaterialInstanceByIndex(0);
		matInst.SetFloat4("myVar", Vec4f(percentage,0,0,0));
	}
}

void Explosion::ScanDamage(float aRange)
{
	// TODO: Add item thingy & Crosshair hitmark

	const float dmg = myDamage.GetDamage();

	Player* player = nullptr;
	if (auto* pollingStation = myGameObject->GetSingletonComponent<PollingStationComponent>())
		player = reinterpret_cast<Player*>(pollingStation->GetPlayerComponent());

	std::vector<OverlapHit> hits;
	auto scene = myGameObject->GetScene();
	if (scene->SphereCastAll(GetTransform().GetPosition(), aRange, myExplosion.targetMask, hits))
	{
		for (int i = 0; i < hits.size(); i++)
		{
			auto& hit = hits[i];

			auto it = std::find(myHits.begin(), myHits.end(), hit.UUID);
			if (it != myHits.end())
				return;

			myHits.emplace_back(hit.UUID);

			if (auto* hp = hit.GameObject->GetComponent<HealthComponent>()) 
			{
				hp->ApplyDamage(dmg, false, nullptr, myDamage.IsCrit() ? eDamageType::Crit : eDamageType::Basic);

				if (player)
					player->GetCrosshair().HitMark();

				const auto dir = GetTransform().GetPosition() - hit.GameObject->GetTransform().GetPosition();
				hp->ApplyKnockback(dir.GetNormalized() * myExplosion.knockback);
			}
		}
	}
}
