#include "pch.h"
#include "ButtonComponent.h"

#include "Engine/UIEventManager/UIEventManager.h"

#include "Engine/GameObject/GameObject.h"
#include "Engine/Reflection/Reflector.h"

#include "Engine/Renderer/GraphicsEngine.h"
#include "Engine/Renderer/WindowHandler.h"

#include "Engine/GameObject/Components/SpriteComponent.h"
#include "Engine/ResourceManagement/Resources/TextureResource.h"

Engine::ButtonComponent::ButtonComponent(GameObject* aGameObject)
	: Component(aGameObject)
{
}

void Engine::ButtonComponent::Awake()
{
	mySprite = myGameObject->GetComponent<SpriteComponent>();
}

void Engine::ButtonComponent::Start()
{
	for (int i = 0; i < myButtonEvents.size(); i++)
	{
		const ButtonEventType type = myButtonEvents[i].myType;
		mySortedButtonEvents[type].push_back(myButtonEvents[i]);
	}

	if (myTransition == +ButtonTransition::ColorTint)
	{
		if (mySprite)
		{
			mySprite->SetColor(myDefaultColor);
			myTransitionProgress = 0.0f;
		}
	}
	else if (myTransition == +ButtonTransition::SpriteSwap)
	{
		if (mySprite)
		{
			mySprite->SetSprite(myDefaultTexture);
		}

		if (myHoverRef && myHoverRef.Get())
		{
			if (myFadingSprite = myHoverRef.Get()->GetComponent<SpriteComponent>())
			{
				myFadingSprite->SetAlpha(0.0f);
			}
		}
	}
}

void Engine::ButtonComponent::OnDisable()
{
	if (myTransition == +ButtonTransition::ColorTint)
	{
		if (mySprite)
		{
			mySprite->SetColor(myDefaultColor);
			myTransitionProgress = 0.0f;
		}
	}
	else if (myTransition == +ButtonTransition::SpriteSwap)
	{
		if (mySprite)
		{
			mySprite->SetSprite(myDefaultTexture);
		}

		if (myFadingSprite)
		{
			myFadingSprite->SetAlpha(0.0f);
		}
	}
}

void Engine::ButtonComponent::Execute(eEngineOrder aOrder)
{
	MouseChecks();
}

const std::vector<Engine::ButtonEvent>& Engine::ButtonComponent::GetEventsFromType(const ButtonEventType aType)
{
	return mySortedButtonEvents[aType];
}

void Engine::ButtonComponent::Reflect(Reflector& aReflector)
{
	Component::Reflect(aReflector);
	aReflector.Reflect(myMouseCheckSize, "Boundaries");
	aReflector.Reflect(myMouseCheckOffset, "Offset");
	aReflector.ReflectEnum(myTransition, "Transition");

	if (myTransition == +ButtonTransition::ColorTint)
	{
		aReflector.Reflect(myDefaultColor, "Default Color", ReflectionFlags::ReflectionFlags_IsColor);
		aReflector.Reflect(myHoverColor, "Hover Color", ReflectionFlags::ReflectionFlags_IsColor);
		aReflector.Reflect(myTransitionDuration, "Transition Duration");
	}
	else if (myTransition == +ButtonTransition::SpriteSwap)
	{
		aReflector.Reflect(myDefaultTexture, "Default Texture");
		aReflector.Reflect(myHoverTexture, "Hover Texture");
		aReflector.Reflect(myHoverRef, "Fading Texture Ref");
		aReflector.Reflect(myTransitionDuration, "Transition Duration");
	}

	aReflector.Reflect(myButtonEvents, "Button Events");
}

void Engine::ButtonComponent::MouseChecks()
{
	const auto& input = GetEngine().GetInputManager();
	const auto& viewport = GetEngine().GetViewport();

	Vec2f mousePos = {
		viewport.myRelativeMousePosition.x,
		viewport.myRelativeMousePosition.y
	};

	//MOUSE INSIDE BUTTON CHECK
	myMouseHovering = IsInsideMe(mousePos);

	//MOUSE LBUTTON CLICKED
	if (myMouseHovering)
	{
		CallButtonEvent(ButtonEventType::ButtonOver);

		if (input.IsMouseKeyDown(Common::MouseButton::Left))
		{
			CallButtonEvent(ButtonEventType::ButtonDown);
		}
		else if (input.WasMouseKeyReleased(Common::MouseButton::Left))
		{
			CallButtonEvent(ButtonEventType::ButtonUp);
		}
	}

	//HOVER ENTER
	if (myHoverEnterable && myMouseHovering)
	{
		myHoverEnterable = false;
		myHoverExitable = true;
		CallButtonEvent(ButtonEventType::ButtonEnter);
		OnButtonEnter();
	}

	//HOVER EXIT
	if (myHoverExitable && !myMouseHovering)
	{
		myHoverEnterable = true;
		myHoverExitable = false;
		CallButtonEvent(ButtonEventType::ButtonExit);
		OnButtonExit();
	}

	if (myTransitionProgress > 0.0f)
	{
		myTransitionProgress -= Time::DeltaTimeUnscaled;
		if (myTransitionProgress <= 0.0f)
		{
			myTransitionProgress = 0.0f;
		}
		
		const float ratio = myTransitionProgress / myTransitionDuration;
		
		if (myTransition == +ButtonTransition::ColorTint)
		{
			if (mySprite)
			{
				Vec4f color = CU::Lerp(myStartColor, myGoalColor, 1.0f - ratio);
				mySprite->SetColor(color);
			}
		}
		else if (myTransition == +ButtonTransition::SpriteSwap)
		{
			if (myFadingSprite)
			{
				float alpha = CU::Lerp(myStartAlpha, myGoalAlpha, 1.0f - ratio);
				myFadingSprite->SetAlpha(alpha);
			}
		}
	}
}

void Engine::ButtonComponent::OnButtonEnter()
{
	if (myTransition == +ButtonTransition::ColorTint)
	{
		if (mySprite)
		{
			myStartColor = mySprite->GetColor();
			myGoalColor = myHoverColor;
			myTransitionProgress = myTransitionDuration;
		}
	}
	else if (myTransition == +ButtonTransition::SpriteSwap)
	{
		if (mySprite && myHoverTexture)
		{
			mySprite->SetSprite(myHoverTexture);
		}

		if (myFadingSprite)
		{
			myStartAlpha = myFadingSprite->GetAlpha();
			myGoalAlpha = 1.0f;
			myTransitionProgress = myTransitionDuration;
		}
	}
}

void Engine::ButtonComponent::OnButtonExit()
{
	if (myTransition == +ButtonTransition::ColorTint)
	{
		if (mySprite)
		{
			myStartColor = mySprite->GetColor();
			myGoalColor = myDefaultColor;
			myTransitionProgress = myTransitionDuration;
		}
	}
	else if (myTransition == +ButtonTransition::SpriteSwap)
	{
		if (mySprite && myHoverTexture)
		{
			mySprite->SetSprite(myDefaultTexture);
		}

		if (myFadingSprite)
		{
			myStartAlpha = myFadingSprite->GetAlpha();
			myGoalAlpha = 0.0f;
			myTransitionProgress = myTransitionDuration;
		}
	}
}

bool Engine::ButtonComponent::IsInsideMe(Vec2f aPos)
{
	aPos = aPos / GetEngine().GetWindowSize();

	auto& targetSize = GetEngine().GetGraphicsEngine().GetWindowHandler().GetTargetSize();
	Vec2f pos = { myGameObject->GetTransform().GetPosition().x + myMouseCheckOffset.x, myGameObject->GetTransform().GetPosition().y + myMouseCheckOffset.y };
	Vec2f size = { myMouseCheckSize.x / targetSize.x, myMouseCheckSize.y / targetSize.y };
	Vec3f scale = myGameObject->GetTransform().GetScale();
	Vec2f scale2D = { scale.x, scale.y };

	GDebugDrawer->DrawRectangle2D(DebugDrawFlags::UI, pos, (size * scale2D) * 0.5f);

	if (aPos.x < pos.x + (size.x * scale.x) * 0.5f &&
		aPos.x > pos.x - (size.x * scale.x) * 0.5f &&
		aPos.y < pos.y + (size.y * scale.y) * 0.5f &&
		aPos.y > pos.y - (size.y * scale.y) * 0.5f)
	{
		return true;
	}
	return false;
}

void Engine::ButtonComponent::CallButtonEvent(ButtonEventType aType)
{
	const ButtonEventType type = aType;
	const bool isInteractionKey = aType == +ButtonEventType::ButtonUp;
	const int size = (int)mySortedButtonEvents[type].size();
	for (int i = size - 1; i >= 0; i--)
	{
		const std::string& eventName = mySortedButtonEvents[type][i].myEventName;
		UIEventManager::CallEvent(eventName, myGameObject, isInteractionKey);
	}

	if (size > 0)
	{
		UIEventManager::OnCallEvent(isInteractionKey);
	}
}

void Engine::ButtonEvent::Reflect(Reflector& aReflector)
{
	aReflector.ReflectEnum(myType, "Event Type");
	aReflector.Reflect(myEventName, "Event Name");
}

void Engine::ButtonComponent::SetDefaultSprite(const std::string& aSpritePath)
{
	myDefaultTexture = GResourceManager->CreateRef<TextureResource>(aSpritePath);
	myDefaultTexture->Load();
}

void Engine::ButtonComponent::SetHoverSprite(const std::string& aSpritePath)
{
	myHoverTexture = GResourceManager->CreateRef<TextureResource>(aSpritePath);
	myHoverTexture->Load();
}