#include "pch.h"
#include "ImageReferencer.h"

#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/SpriteComponent.h"

void ImageReferencer::Start()
{
	if (myImageRef && myImageRef.IsValid())
	{
		mySprite = myImageRef.Get()->GetComponent<Engine::SpriteComponent>();
	}

	InstantDisable();
}

void ImageReferencer::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myImageRef, "Image Reference");
	aReflector.Reflect(myWillFade, "Fade Image");
	
	if (myWillFade)
	{
		aReflector.Reflect(myFadeDuration, "Fade Duration");
		aReflector.Reflect(myStartSize, "Start Scale");
		aReflector.Reflect(myEndSize, "End Scale");
	}
}

void ImageReferencer::OnEnable()
{
	InstantDisable();
}

void ImageReferencer::OnDisable()
{
	InstantDisable();
}

void ImageReferencer::Execute(Engine::eEngineOrder aOrder)
{
	if (myImageRef && myImageRef.IsValid())
	{
		auto gameObject = myImageRef.Get();
		if (myWillFade && myFadeProgress > 0.0f)
		{
			myFadeProgress -= Time::DeltaTime;
			if (myFadeProgress <= 0.0f)
			{
				myFadeProgress = 0.0f;
			}
			const float ratio = 1.0f - (myFadeProgress / myFadeDuration);

			float scale = CU::Lerp(myStartScale, myGoalScale, ratio);
			auto& transform = gameObject->GetTransform();
			transform.SetScale(scale);

			if (mySprite)
			{
				float alpha = CU::Lerp(myStartAlpha, myGoalAlpha, ratio);
				mySprite->SetAlpha(alpha);
			}
		}
	}
}

void ImageReferencer::InstantDisable()
{
	if (myImageRef && myImageRef.IsValid())
	{
		auto gameObject = myImageRef.Get();
		auto& transform = gameObject->GetTransform();
		transform.SetScale(myStartSize);

		if (mySprite)
		{
			mySprite->SetAlpha(0.0f);
		}
	}
}

void ImageReferencer::EnableImage()
{
	if (myImageRef && myImageRef.IsValid())
	{
		if (myWillFade)
		{
			auto gameObject = myImageRef.Get();
			auto& transform = gameObject->GetTransform();
			myStartScale = transform.GetScale().x;
			myGoalScale = myEndSize;

			if (mySprite)
			{
				myStartAlpha = mySprite->GetAlpha();
				myGoalAlpha = 1.0f;
			}
			myFadeProgress = myFadeDuration;
		}
	}
}

void ImageReferencer::DisableImage()
{
	if (myImageRef && myImageRef.IsValid())
	{
		if (myWillFade)
		{
			auto gameObject = myImageRef.Get();
			auto& transform = gameObject->GetTransform();
			myStartScale = transform.GetScale().x;
			myGoalScale = myStartSize;

			if (mySprite)
			{
				myStartAlpha = mySprite->GetAlpha();
				myGoalAlpha = 0.0f;
			}
			myFadeProgress = myFadeDuration;
		}
	}
}
