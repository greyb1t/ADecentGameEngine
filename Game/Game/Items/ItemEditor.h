#pragma once

class ItemEffectBase;

class Item;
class ItemEditor
{
public:
	ItemEditor();
	~ItemEditor() = default;

	void Update();

	void LoadItems();
	void SaveItems(); 
private:
	void LoadEffect(Item& anItem, nlohmann::json& aJson);
	void CreateNewItem();

	void UpdateItem(Item& anItem);
	bool UpdateEffect(ItemEffectBase* anEffect, int anIndex);

	int mySelectedIndex = 0;
	bool myShouldDisplay = false;
	std::set<int> myUsedIDs;

};

