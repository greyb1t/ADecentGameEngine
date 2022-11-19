#include "pch.h"
#include "CrosshairUI.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/SpriteComponent.h"

CrosshairUI::CrosshairUI(GameObject* aGameObject)
	: Component(aGameObject)
{

}

CrosshairUI::~CrosshairUI()
{
}

void CrosshairUI::Start()
{
	myVerified = InitLines() && InitHitMark();
}

void CrosshairUI::Execute(Engine::eEngineOrder aOrder)
{
	if (!myVerified)
		return;
}

void CrosshairUI::MoveLine(int aIndex, float aAngle, float aDistance)
{
	myLines[aIndex]->GetTransform().SetPosition(S2W3({ aDistance * cosf(aAngle * Math::Deg2Rad), aDistance * sinf(aAngle * Math::Deg2Rad) }));
	myLines[aIndex]->GetTransform().SetRotation(Quatf(Vec3f(0,0, (-90 + aAngle) * Math::Deg2Rad)));
}

void CrosshairUI::SetHitMark(bool aIsActive, float aSize, float aAlpha)
{
	if (!myVerified)
		return;

	myHitMarkSprite->SetActive(aIsActive);

	if (!aIsActive)
		return;

	myHitMark->GetTransform().SetScaleLocal(Vec3f(aSize, aSize, 1));
	myHitMarkSprite->SetAlpha(aAlpha);
}

void CrosshairUI::SetVisible(bool anIsVisible)
{
	for (auto* line : myLines)
	{
		if (line)
		{
			line->SetActive(anIsVisible);
		}
	}
}

bool CrosshairUI::InitLines()
{
	auto* linesParent = GetTransform().GetChildByGameObjectName("Lines");
	if (!linesParent)
		return false;

	auto children = linesParent->GetTransform().GetChildren();

	if (children.size() != 4)
		return false;

	int i = 0;
	for (auto& child : children)
	{
		myLines[i++] = child->GetGameObject();
	}
	return true;
}

bool CrosshairUI::InitHitMark()
{
	myHitMark = GetTransform().GetChildByGameObjectName("HitMark");
	if (!myHitMark)
		return false;

	myHitMarkSprite = myHitMark->GetComponent<Engine::SpriteComponent>();
	if (!myHitMarkSprite)
		return false;

	myHitMarkSprite->SetActive(false);

	return true;
}

Vec2f S2W(Vec2f aScreenPos)
{
	return Vec2f(0.5f + aScreenPos.x / 1920.f, 0.5f + aScreenPos.y / 1080.f);
}

Vec3f S2W3(Vec2f aScreenPos)
{
	const Vec2f pos = S2W(aScreenPos);
	return { pos.x,pos.y, 0};
}
