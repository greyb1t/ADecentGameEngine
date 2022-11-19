#pragma once

#include "MenuComponent.h"

class Credits : public MenuComponent
{
	COMPONENT(Credits, "Credits");
public:
	Credits() = default;
	~Credits() = default;

	void Start() override;

private:

};
