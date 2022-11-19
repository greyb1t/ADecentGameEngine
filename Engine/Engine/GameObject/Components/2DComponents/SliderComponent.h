#pragma once

#include "Engine/GameObject/Components/Component.h"
#include "Engine/GameObject/GameObjectRef.h"

namespace Engine
{
	class ButtonComponent;

	BETTER_ENUM(SliderEventType, int,
		SliderKnobDown,
		SliderKnobUp,
		SliderKnobOver,
		SliderKnobEnter,
		SliderKnobExit,

		SliderValueChanged
	);

	struct SliderEvent : public Reflectable
	{
		SliderEventType myType = SliderEventType::SliderKnobDown;
		std::string myEventName = "";

		void Reflect(Reflector& aReflector);
	};

	// USING SOME OBJECT SPECIFIC FUNCTIONALITY (NOT FULLY COPYABLE)
	class SliderComponent : public Component
	{
	public:
		COMPONENT(SliderComponent, "SliderComponent");

		SliderComponent() = default;
		SliderComponent(GameObject* aGameObject);

		void Awake() override;

		void Execute(eEngineOrder aOrder) override;

		void Reflect(Reflector& aReflector) override;
		void SetValue(const float aValue);

	private:
		bool IsInsideKnob(const Vec2f& aPos);
		void MoveKnob(const Vec2f& aPos);

		void MouseChecks();
		void DebugDraw();

		void CallSliderEvent(SliderEventType aType);

		void SetWidths();

	private:
		std::vector<SliderEvent> mySliderEvents;
		std::map<SliderEventType, std::vector<SliderEvent>> mySortedSliderEvents;

		GameObjectRef myKnob;
		GameObjectRef myFillbar;

		ButtonComponent* myKnobButton = nullptr;
		SpriteComponent* myFillbarSprite = nullptr;

		Vec2f myDragOffset = { 0, 0 };
		bool myIsMovingKnob = false;

		float myWidth = 0.0f;
		float myWidthNormalized = 0.0f;

		float myMinValue = 0.0f;
		float myMaxValue = 0.0f;

		float myValue = 0.f;
		float myOldValue = 0.f;

		bool myMouseHovering = false;
		bool myHoverExitable = false;
		bool myHoverEnterable = true;

	};
}