
#include "Engine/GameObject/Components/Component.h"
#include "Engine/GameObject/GameObjectRef.h"
#include "Behaviours/HandCrawler.h"
#include "FinalBossHand.h"
#include "FinalBossDamage.h"

namespace FB
{
	BETTER_ENUM(LeftHandBehaviours, int,
		Idle,
		Death,
		HandPunch,
		HandSlam,
		Crawler,
		Enraged);

	class Behaviour;
	class FinalBoss;

	struct HandPunchDesc : public Engine::Reflectable
	{
		float myHitForce = 1000.f;
		AnimationCurveRef mySpeedCurve;

		FinalBossDamage myDamage;

		VFXRef myPunchTrailVFX;
		VFXRef myPunchCollidedVFX;

		float myTimeToReachPlayerSec = 1.f;

		void Reflect(Engine::Reflector& aReflector) override
		{
			aReflector.Reflect(myHitForce, "Hit Force");
			aReflector.Reflect(mySpeedCurve, "Move to player Speed Curve");
			aReflector.Reflect(myPunchTrailVFX, "Punch Trail VFX");
			aReflector.Reflect(myPunchCollidedVFX, "Punch Collided VFX");
			aReflector.Reflect(myDamage, "Damage");
			aReflector.Reflect(myTimeToReachPlayerSec, "Time To Reach Player Sec ");
		}
	};

	struct HandSlamDesc : public Engine::Reflectable
	{
		float mySearchDistanceAbovePlayer = 1000.f;

		AnimationCurveRef mySearchSpeedCurve;
		AnimationCurveRef mySlamSpeedCurve;

		VFXRef myHitGroundVFX;

		FinalBossDamage myDamage;

		float myFallDurationSec = 0.5f;

		void Reflect(Engine::Reflector& aReflector) override
		{
			aReflector.Reflect(mySearchDistanceAbovePlayer, "Search Distance Above Player");
			aReflector.Reflect(mySearchSpeedCurve, "Search Speed Curve");
			aReflector.Reflect(mySlamSpeedCurve, "Slam Speed Curve");
			aReflector.Reflect(myHitGroundVFX, "Hit Ground VFX");
			aReflector.Reflect(myDamage, "Damage");
			aReflector.Reflect(myFallDurationSec, "Fall Duration Sec");
		}
	};

	class LeftPunchHand : public FinalBossHand
	{
	public:
		COMPONENT(LeftPunchHand, "FB Left Punch Hand");

		LeftPunchHand();
		LeftPunchHand(GameObject* aGameObject);
		virtual ~LeftPunchHand();

		GameObject* GetColliderGameObject();

		const HandPunchDesc& GetHandPunchDesc() const;
		const HandSlamDesc& GetHandSlamDesc() const;

		void OnEnterPhase(const FinalBossPhase aPhase) override;

		bool IsDemolished() const override;

		float GetMaxAllowedDistanceAbovePortal() override;

	protected:
		void Awake() override;
		void Start() override;

		void Execute(Engine::eEngineOrder aOrder) override;

		void Reflect(Engine::Reflector& aReflector) override;

		void OnDeath() override;

	private:
		HandPunchDesc myHandPunchDesc;
		HandSlamDesc myHandSlamDesc;

		TickTimer myIntroBezierDelayTimer;
		TickTimer myIntroBezierTimer;

		AnimationCurveRef myIntroSplineSpeedCurve;
	};
}