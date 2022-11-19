#pragma once

#include "Component.h"
#include "Engine/Animation/AnimationController.h"
#include "Engine/Animation/AnimationPair.h"
#include "Engine/Animation/State/AnimationMachine.h"
#include "Engine\ResourceManagement\ResourceObserver.h"
#include "Engine\ResourceManagement\ResourceRef.h"
#include "Engine/GameObject/GameObjectRef.h"

namespace Engine
{
	class ModelComponent;
	class BlendTree1D;
	class NormalAnimation;

	class AnimatorComponent : public Component, public ResourceObserver
	{
	public:
		// Animations are done later because in RendererScene::Update we cull and do the render checks
		// that the animations depend on
		COMPONENT_MASK(AnimatorComponent, "AnimatorComponent", LATE_UPDATE);

		AnimatorComponent() = default;
		AnimatorComponent(
			GameObject* aGameObject,
			ModelComponent& aSkinnedModelComponent,
			const std::string& aName = "");

		AnimatorComponent(const AnimatorComponent& aOther);

		virtual ~AnimatorComponent();

		void Awake() override;

		void InitFromJson(const std::string& aPath);

		void Execute(eEngineOrder aOrder) override;
		void ExecuteParallelized(eEngineOrder aOrder) override;
		void PostExecute() override;

		AnimationController& GetController();

		void ReloadAnimation();

		void OnResourceReloaded() override;

		void Reflect(Reflector& aReflector) override;

		bool HasMachine() const;

	private:
		void LoadAnimationFromJson(const std::string& aPath);
		void LoadAnimationFromJson(const AnimationMachineRef& aAnimation);

	private:
		std::string myJsonPath;

		ModelComponent* mySkinnedModelComponent = nullptr;

		AnimationMachineRef myAnimationResource;
		AnimationController myController;
		AnimationMachine myMachine;

		std::string myName;

		GameObjectRef myRootMotionTarget;
	};
}