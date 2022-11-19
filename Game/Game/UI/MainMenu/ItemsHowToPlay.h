#pragma once

#include "MenuComponent.h"

namespace Engine
{
	class TextComponent;
	class SpriteComponent;
}

static const Vec4f COLOR_COMMON		= Vec4f(185.0f / 255.0f, 255.0f / 255.0f, 123.0f / 255.0f, 1.0f);
static const Vec4f COLOR_RARE		= Vec4f(68.0f / 255.0f, 216.0f / 255.0f, 243.0f / 255.0f, 1.0f);
static const Vec4f COLOR_LEGENDARY	= Vec4f(214.0f / 255.0f, 50.0 / 255.0f, 255.0f / 255.0f, 1.0f);

class ItemsHowToPlay : public MenuComponent
{
	COMPONENT(ItemsHowToPlay, "ItemsHowToPlay");
public:
	ItemsHowToPlay() = default;
	~ItemsHowToPlay() = default;

	void Start() override;
	void Reflect(Engine::Reflector& aReflector) override;

	void Execute(Engine::eEngineOrder aOrder) override;

	void OnEnable() override;
	void OnDisable() override;

private:
	void InitRefs();
	void InitItemDescs();
	void InitButtonCallbacks();
	
	void OnEnterItem(GameObject* aButtonObject);
	void OnExitItem(GameObject* aButtonObject);

	void AddItemDesc(Item* anItem);

private:
	struct ItemDesc
	{
		Transform* myTransform;
		std::string myName = "";
		std::string myShortDesc = "";
		std::string myLongDesc = "";
		std::string mySpritePath = "";
	};
	std::vector<ItemDesc> myItemDescs;
	GameObjectPrefabRef myItemButtonPrefab;

	Engine::GameObjectRef myImageRef;
	Engine::GameObjectRef myTitleRef;
	Engine::GameObjectRef myDescriptionRef;

	Engine::SpriteComponent* myImageSprite = nullptr;
	Engine::TextComponent* myTitleText = nullptr;
	Engine::TextComponent* myDescriptionText = nullptr;

	Vec2f myStartPos = { 0.2f, 0.35f };
	Vec2f myPadding = { 165.0f, 120.0f };
	int myColumnSize = 8;
	int myDebugAmount = 24;
	bool myDebugDraw = false;

};