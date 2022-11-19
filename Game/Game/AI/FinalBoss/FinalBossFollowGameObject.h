#pragma once

#include "Engine/GameObject/Components/Component.h"

namespace FB
{
	class FinalBossFollowGameObject : public Component
	{
	public:
		COMPONENT(FinalBossFollowGameObject, "FB Follow GameObject");

		void Start() override;

		void Execute(Engine::eEngineOrder aOrder) override;

		void Reflect(Engine::Reflector& aReflector) override;

	private:
		Engine::GameObjectRef myTargetGameObject;

		Vec3f myPositionOffset;
	};
}