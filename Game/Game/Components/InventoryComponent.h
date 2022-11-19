#pragma once
#include "Engine\GameObject\Components\Component.h"
#include "Items/Item.h"

namespace Engine
{
	class SpriteComponent;
	class TextComponent;
}

class PauseMenuComponent;

struct ItemImage
{
	Engine::SpriteComponent* sprite = nullptr;
	int ID = -1;
};

class InventoryComponent : public Component
{
	COMPONENT(InventoryComponent, "Inventory");
public:
	virtual ~InventoryComponent(); 
	void Start() override;
	void Reflect(Engine::Reflector& aReflector) override;
	void AddItem(const Item& anItem);
	void Clear(); //Empties the inventory completely, across different scenes
	const std::vector<Item*>& GetItems() const;

	Item* GetSelectedItem(Vec2f aMousePos);
	const Vec3f& GetItemPosition(Item* aItem);

	void ShowItems(bool aShouldShow);

private:
	CU::Vector3f ToTextSpace(const CU::Vector3f& aPosition);
	void RemoveItemAtIndex(int anIndex);
	void AddSprite(const Item& anItem);

	std::vector<Item*> myItems;
	std::vector<ItemImage> myImages;
	std::vector<Engine::TextComponent*> myText;

	CU::Vector2f myItemStartPos = { 0.555f, 0.23f };
	CU::Vector2f myItemSpacing = { 0.055f, 0.12f };
	int myRowMaxLength = 4;
	int myRowLength = 0;
	int myRowHeight = 0;

	PauseMenuComponent* myPauseMenu = nullptr;

	VFXRef myVFXRef;
};

