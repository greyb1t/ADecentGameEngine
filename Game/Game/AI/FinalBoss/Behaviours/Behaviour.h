#pragma once

namespace Engine
{
	class Reflector;
}

namespace FB
{
	class FinalBossHand;

	class Behaviour
	{
	public:
		Behaviour();
		virtual ~Behaviour();

		virtual void Update() = 0;
		virtual bool IsFinished() const = 0;

		virtual void OnEnter() {}
		virtual void OnExit() {}

		virtual void Reflect(Engine::Reflector& aReflector) {}

		virtual bool IsAllowedToInterrupt() const { return false; }
		virtual bool IsAllowedToMoveExternally() const { return true; }

		virtual void OnCollisionEnter(GameObject& aGameObject) {}
		virtual void OnCollisionStay(GameObject& aGameObject) {}

		// protected:
		// 	FinalBossHand& myHand;
	};

	using BehaviourEnumValue = int;
}