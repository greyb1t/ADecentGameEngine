#pragma once

namespace FB
{
	class BaseFinalBossState
	{
	public:
		virtual void Update() {}

		virtual bool IsFinished() = 0;

		virtual void OnEnter() {}
		virtual void OnExit() {}

		virtual void Reflect(Engine::Reflector& aReflector) {}
	};

	/*
	template <typename T>
	class FinalBossState : public BaseFinalBossState
	{
	public:
		FinalBossState(T& aParent);
		virtual ~FinalBossState() = default;

	protected:
		T& myParent;
	};

	template <typename T>
	FB::FinalBossState<T>::FinalBossState(T& aParent)
		: myParent(aParent)
	{
	}
	*/
}