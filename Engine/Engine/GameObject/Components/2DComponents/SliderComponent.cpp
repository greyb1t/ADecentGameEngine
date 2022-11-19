#include "pch.h"
#include "SliderComponent.h"

#include "Engine/UIEventManager/UIEventManager.h"

#include "Engine/Reflection/Reflector.h"
#include "Engine/GameObject/GameObject.h"

#include "Engine/Renderer/GraphicsEngine.h"
#include "Engine/Renderer/WindowHandler.h"

#include "ButtonComponent.h"
#include "../SpriteComponent.h"

Engine::SliderComponent::SliderComponent(GameObject* aGameObject) : Component(aGameObject)
{ }

void Engine::SliderComponent::Awake()
{
	for (int i = 0; i < mySliderEvents.size(); i++)
	{
		const SliderEventType type = mySliderEvents[i].myType;
		mySortedSliderEvents[type].push_back(mySliderEvents[i]);
	}

	if (myKnob && myKnob.Get())
	{
		myKnobButton = myKnob.Get()->GetComponent<ButtonComponent>();
	}

	SetWidths();

	myOldValue = myValue;
}

void Engine::SliderComponent::Execute(eEngineOrder aOrder)
{
	MouseChecks();
	DebugDraw();
}

void Engine::SliderComponent::MouseChecks()
{
	const auto& input = GetEngine().GetInputManager();
	const auto& viewport = GetEngine().GetViewport();

	Vec2f mousePos = {
		viewport.myRelativeMousePosition.x,
		viewport.myRelativeMousePosition.y
	};

	myMouseHovering = IsInsideKnob(mousePos);

	//MOUSE LBUTTON CLICKED
	if (myMouseHovering)
	{
		CallSliderEvent(SliderEventType::SliderKnobOver);

		if (input.IsMouseKeyDown(Common::MouseButton::Left))
		{
			if (myKnob && myKnob.Get())
			{
				const auto& position = myKnob.Get()->GetTransform().GetPosition();
				myDragOffset = Vec2f(position.x, position.y) - viewport.myRelativeNormalizedMousePosition;
			}

			myIsMovingKnob = true;
			CallSliderEvent(SliderEventType::SliderKnobDown);
		}
	}

	if (input.WasMouseKeyReleased(Common::MouseButton::Left) && myIsMovingKnob)
	{
		myIsMovingKnob = false;
		CallSliderEvent(SliderEventType::SliderKnobUp);
	}

	//HOVER ENTER
	if (myHoverEnterable && myMouseHovering)
	{
		myHoverEnterable = false;
		myHoverExitable = true;
		CallSliderEvent(SliderEventType::SliderKnobEnter);
	}

	//HOVER EXIT
	if (myHoverExitable && !myMouseHovering)
	{
		myHoverEnterable = true;
		myHoverExitable = false;
		CallSliderEvent(SliderEventType::SliderKnobExit);
	}

	if (myIsMovingKnob)
	{
		MoveKnob(viewport.myRelativeNormalizedMousePosition + myDragOffset);
	}

	const auto& rnmp = viewport.myRelativeNormalizedMousePosition;
	if (rnmp.x < 0.0f || rnmp.x > 1.0f || rnmp.y < 0.0f || rnmp.y > 1.0f)
	{
		myIsMovingKnob = false;
	}

	if (myValue != myOldValue)
	{
		CallSliderEvent(SliderEventType::SliderValueChanged);
		myOldValue = myValue;
	}
}

void Engine::SliderComponent::DebugDraw()
{
	if (!myFillbarSprite)
		return;

	Vec2f targetSize = GetEngine().GetGraphicsEngine().GetWindowHandler().GetTargetSize().CastTo<float>();

	const auto& position = myGameObject->GetTransform().GetPosition();
	Vec2f half_extends = myFillbarSprite->GetSizeWithinUVRect().CastTo<float>();
	half_extends = half_extends / targetSize;
	half_extends = half_extends * .5f;

	GDebugDrawer->DrawRectangle2D(DebugDrawFlags::UI, { position.x, position.y }, half_extends, 0.0f, Vec4f(1,0,0,1));
}

void Engine::SliderComponent::CallSliderEvent(SliderEventType aType)
{
	if (myKnob && myKnob.Get())
	{
		const SliderEventType type = aType;
		const int size = (int)mySortedSliderEvents[type].size();
		for (int i = size - 1; i >= 0; i--)
		{
			const std::string& eventName = mySortedSliderEvents[type][i].myEventName;
			UIEventManager::CallEvent(eventName, myKnob.Get(), myValue,
				aType == +SliderEventType::SliderKnobUp
			);
		}
		UIEventManager::OnCallEvent(aType == +SliderEventType::SliderKnobUp);
	}
}

void Engine::SliderComponent::SetWidths()
{
	if (myFillbar && myFillbar.Get())
	{
		myFillbarSprite = myFillbar.Get()->GetComponent<SpriteComponent>();

		Vec2f targetSize = GetEngine().GetGraphicsEngine().GetWindowHandler().GetTargetSize().CastTo<float>();

		const auto& position = myGameObject->GetTransform().GetPosition();
		Vec2f size = myFillbarSprite->GetSizeWithinUVRect().CastTo<float>();
		size = size * .5f;

		myWidth = size.x;

		size = size / targetSize;
		myWidthNormalized = size.x;

		myMinValue = position.x - myWidthNormalized;
		myMaxValue = position.x + myWidthNormalized;
	}
}

bool Engine::SliderComponent::IsInsideKnob(const Vec2f& aPos)
{
	if (!myKnobButton)
		return false;

	return myKnobButton->IsInsideMe(aPos);
}

void Engine::SliderComponent::MoveKnob(const Vec2f& aPos)
{
	if (myKnob && myKnob.Get())
	{
		auto& transform = myKnob.Get()->GetTransform();
		auto& oldPosition = transform.GetPosition();
		auto& barPosition = myGameObject->GetTransform().GetPosition();

		Vec2f newPosition = { aPos.x, oldPosition.y };

		newPosition.x = CU::Clamp(barPosition.x - myWidthNormalized, barPosition.x + myWidthNormalized, newPosition.x);
		newPosition.y = CU::Clamp(0.0f, 1.0f, newPosition.y);

		transform.SetPosition({ newPosition.x, newPosition.y, oldPosition.z });

		float m = myMaxValue - myMinValue;
		float v = myMaxValue - newPosition.x;
		myValue = 1.0f - (v / m);

		if (myFillbarSprite)
		{
			const auto& uv = myFillbarSprite->GetUV();
			const float width = uv.myEnd.x - uv.myStart.x;
			myFillbarSprite->GetMaterialInstance().SetFloat4("value1", Vec4f(width * myValue, 0.0f, 0.0f, 0.0f));
		}
	}
}

void Engine::SliderComponent::Reflect(Reflector& aReflector)
{
	aReflector.Reflect(myKnob, "Knob");
	aReflector.Reflect(myFillbar, "Fillbar");
	aReflector.Reflect(myValue, "Value", ReflectionFlags::ReflectionFlags_ReadOnly);
	aReflector.Reflect(mySliderEvents, "Events");
}

void Engine::SliderComponent::SetValue(const float aValue)
{
	if (myKnob && myKnob.Get())
	{
		myValue = CU::Clamp(0.0f, 1.0f, aValue);
		
		auto& barPosition = myGameObject->GetTransform().GetPosition();

		if (myMinValue == 0.0f || myMaxValue == 0.0f)
		{
			SetWidths();
		}

		float m = myMaxValue - myMinValue;
		float x = myMinValue + (myValue * m);

		auto& transform = myKnob.Get()->GetTransform();
		auto& oldPosition = transform.GetPosition();
		transform.SetPosition({ x, oldPosition.y, oldPosition.z });

		if (myFillbarSprite)
		{
			const auto& uv = myFillbarSprite->GetUV();
			const float width = uv.myEnd.x - uv.myStart.x;
			myFillbarSprite->GetMaterialInstance().SetFloat4("value1", Vec4f(width * myValue, 0.0f, 0.0f, 0.0f));
		}
	}
}

void Engine::SliderEvent::Reflect(Reflector& aReflector)
{
	aReflector.ReflectEnum(myType, "Event Type");
	aReflector.Reflect(myEventName, "Event Name");
}