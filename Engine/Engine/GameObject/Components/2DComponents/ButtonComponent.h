#pragma once

#include "Engine/GameObject/Components/Component.h"
#include "Engine/ResourceManagement/ResourceRef.h"
#include "Engine/GameObject/GameObjectRef.h"

namespace Engine
{
	BETTER_ENUM(ButtonTransition, int,
		None,
		ColorTint,
		SpriteSwap
		);

	BETTER_ENUM(ButtonEventType, int,
		ButtonDown = 0,
		ButtonUp,
		ButtonOver,
		ButtonEnter,
		ButtonExit
		);

	struct ButtonEvent : public Reflectable
	{
		ButtonEventType myType = ButtonEventType::ButtonDown;
		std::string myEventName = "";

		void Reflect(Reflector& aReflector);
	};

	class ButtonComponent : public Component
	{
		COMPONENT(ButtonComponent, "ButtonComponent");
	public:

		ButtonComponent() = default;
		ButtonComponent(GameObject* aGameObject);

		void Awake() override;
		void Start() override;

		void Reflect(Reflector& aReflector) override;
		
		void Execute(eEngineOrder aOrder) override;

		const std::vector<ButtonEvent>& GetEventsFromType(const ButtonEventType aType);

		bool IsInsideMe(Vec2f aPos);

		void SetDefaultSprite(const std::string& aSpritePath);
		void SetHoverSprite(const std::string& aSpritePath);

	private:
		void MouseChecks();

		void OnButtonEnter();
		void OnButtonExit();

		void OnDisable() override;

		void CallButtonEvent(ButtonEventType aType);

	private:
		std::vector<ButtonEvent> myButtonEvents;
		std::map<ButtonEventType, std::vector<ButtonEvent>> mySortedButtonEvents;

		Vec2f myMouseCheckOffset = Vec2f(0.f, 0.f);
		Vec2f myMouseCheckSize = Vec2f(100.f, 100.f);
		
		bool myMouseHovering = false;
		bool myHoverExitable = false;
		bool myHoverEnterable = true;

		ButtonTransition myTransition = ButtonTransition::None;

		Vec4f myDefaultColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		Vec4f myHoverColor = { 0.4f, 0.4f, 0.4f, 1.0f };
		
		Vec4f myStartColor = {};
		Vec4f myGoalColor = {};
		
		float myTransitionProgress = 0.0f;
		float myTransitionDuration = 0.25f;

		TextureRef myDefaultTexture;
		TextureRef myHoverTexture;
		GameObjectRef myHoverRef;
		SpriteComponent* mySprite = nullptr;
		SpriteComponent* myFadingSprite = nullptr;

		float myStartAlpha = 1.0f;
		float myGoalAlpha = 1.0f;

	};
}