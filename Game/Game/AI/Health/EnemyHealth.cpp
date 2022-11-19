#include "pch.h"
#include "EnemyHealth.h"

#include "AI/BehaviorTree/Base/BT_Blackboard.h"
#include "AI/Enemies/Base/EnemyBase.h"
#include "AI/Explosion/GeneralExplosion.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "AI/VFX/EnemyVFXRef.h"

#include "Components/HealthComponent.h"
#include "Engine/CameraShake/CameraShakeManager.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/CameraShakeComponent.h"
#include "Engine/GameObject/Components/ModelComponent.h"
#include "Engine/GameObject/Components/VFXComponent.h"

#include "Engine\Reflection\Reflector.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"

#include "Items/ItemManager.h"

EnemyHealth::EnemyHealth()
{
	myEyeEffectObject = nullptr;
	myEyeObject = nullptr;
}

EnemyHealth::EnemyHealth(GameObject* aGameObject, float aMaxHealth) :
	Component(aGameObject),
	myMaxHealth(aMaxHealth)
{
}

EnemyHealth::~EnemyHealth()
{
}

void EnemyHealth::Init(BT_Blackboard* aBlackboard, std::function<void()> aOnDeathFunction)
{
	myBlackboard = aBlackboard;
	myDeathFunction = aOnDeathFunction;
}

void EnemyHealth::Reflect(Engine::Reflector& aReflector)
{

}

void EnemyHealth::Start()
{
	myHealthComponent = myGameObject->AddComponent<HealthComponent>(myMaxHealth);

	myHealthComponent->ObserveDamage(std::bind(&EnemyHealth::OnDamage, this, std::placeholders::_1));
	myHealthComponent->ObserveDeath(std::bind(&EnemyHealth::OnDeath, this, std::placeholders::_1));
}

void EnemyHealth::Execute(Engine::eEngineOrder aOrder)
{
	if (myBlackboard->getBool("IsDead"))
	{
		if (myGameObject)
		{
			myDeathSinkTimer += Time::DeltaTime;
			if (myDeathSinkTimer > myDeathSinkTime * 0.5f)
			{
				auto obj = myGameObject->GetScene()->AddGameObject<GameObject>();
				obj->GetTransform().SetParent(&GetTransform());
				obj->GetTransform().SetPositionLocal(Vec3f(0.f, 0.f, 0.f));

				const auto& vfxRef = myGameObject->GetSingletonComponent<EnemyVFXRef>()->GetEnemyDeathDecay();
				myDecayVFX = obj->AddComponent<Engine::VFXComponent>(vfxRef->Get());
				myDecayVFX->Play();
				myDecayVFX->AutoDestroy();
			}

			if (myDeathSinkTimer > myDeathSinkTime)
			{
				Vec3f pos = myGameObject->GetTransform().GetPosition();
				pos.y -= 25.f * Time::DeltaTime;
				myGameObject->GetTransform().SetPosition(pos);
			}
		}

		myDeathTimer += Time::DeltaTime;
		if (myDeathTimer > myDeathDuration)
		{
			if (myEyeObject != nullptr)
			{
				myEyeObject->Destroy();
			}

			if (myEyeEffectObject != nullptr)
			{
				myEyeEffectObject->Destroy();
			}

			myGameObject->Destroy();
		}
	}

	if (myActivateOnDamgeTimer)
	{
		myOnDamageTimer += Time::DeltaTime;
	}
	if (myOnDamageTimer >= myOnDamageDuration)
	{
		myActivateOnDamgeTimer = false;
		myOnDamageTimer = 0.f;
		auto& materialInstance = myGameObject->GetComponent<Engine::ModelComponent>()->GetMeshMaterialInstanceByIndex(0);
		auto values = materialInstance.GetFloat4("emissiveStr");
		values.z = 0.f;
		values.x = 5.f;
		materialInstance.SetFloat4("emissiveStr", values);
	}
}

void EnemyHealth::OnDamage(float aDamage)
{
	if (myBlackboard->getBool("IsDead"))
		return;

	auto& materialInstance = myGameObject->GetComponent<Engine::ModelComponent>()->GetMeshMaterialInstanceByIndex(0);
	auto values = materialInstance.GetFloat4("emissiveStr");
	values.z = 1.f;
	values.x = 20.f;
	materialInstance.SetFloat4("emissiveStr", values);
	myActivateOnDamgeTimer = true;

	LOG_INFO(LogType::Jesper) << "Enemy Damaged";
	if (myBlackboard->GetAnimationController() && myBlackboard->GetAnimationController()->HasTrigger("Damaged"))
		Enemy::EnemyTrigger(*myBlackboard->GetAnimationController(), "Damaged");


}

void EnemyHealth::OnDeath(float aDamage)
{
	if (myBlackboard->getBool("IsDead"))
		return;

	myGameObject->GetSingletonComponent<PollingStationComponent>()->EnemyCounterDecrease();
	{
		LOG_INFO(LogType::Items) << "OnEnemyKill Called";

		ItemEventData data;
		data.targetObject = myGameObject;
		data.damage = aDamage;
		data.hitPos = GetTransform().GetPosition();

		Main::GetItemManager().PostEvent(eItemEvent::OnEnemyKill, &data);
	}

	//LOG_INFO(LogType::Jesper) << myHealthComponent->GetHealth();

	if (myBlackboard->GetAnimationController() && myBlackboard->GetAnimationController()->HasTrigger("Death"))
	{
		Enemy::EnemyTrigger(*myBlackboard->GetAnimationController(), "Death");
		if (myBlackboard->hasBool("IsDead"))
		{
			myBlackboard->setBool("IsDead", true);
		}
	}

	myBlackboard->GetOwner()->SetTag(eTag::DEFAULT);
	myBlackboard->GetOwner()->SetLayer(eLayer::DEFAULT);
	if (myDeathFunction)
	{
		myDeathFunction();

		auto enemyBase = myGameObject->GetComponent<EnemyBase>();
		if (enemyBase->GetEnemyDeathType() == EnemyDeathType::Explode)
		{
			auto explosionObj = myGameObject->GetScene()->AddGameObject<GameObject>();
			explosionObj->GetTransform().SetPosition(myGameObject->GetTransform().GetPosition());
			const auto& vfxRef = myGameObject->GetSingletonComponent<EnemyVFXRef>()->GetEnemyFlyDeathExplosion();
			explosionObj->AddComponent<GeneralExplosion>(vfxRef->Get());

			const float distance = (myGameObject->GetScene()->GetMainCamera().GetTransform().GetPosition() - myGameObject->GetTransform().GetPosition()).Length();

			const auto mainCam = myGameObject->GetScene()->GetMainCameraGameObject();
			if (const auto shakeComponent = mainCam->GetComponent<Engine::CameraShakeComponent>())
			{
				shakeComponent->AddPerlinShakeByDistance("Normal Enemy Death", 5000.f, distance);
			}

			myGameObject->Destroy(); // flying should explode 
		}
	}
	else
	{
		int hej = -1;
	}
}

void EnemyHealth::SetMaxHealth(const float aHealth)
{
	myHealthComponent->SetMaxHealth(aHealth);
}

void EnemyHealth::Kill()
{
	myHealthComponent->ApplyDamage(1000000.f);
}

void EnemyHealth::SetEyeGameObject(GameObject* aGameobject)
{
	myEyeObject = aGameobject;
}

void EnemyHealth::SetEyeEffectGameObject(GameObject* aGameobject)
{
	myEyeEffectObject = aGameobject;
}

//void EnemyHealth::CallDeathFunction()
//{
//	myDeathFunction();
//}

HealthComponent* EnemyHealth::GetHealthComponent() const
{
	return myHealthComponent;
}



