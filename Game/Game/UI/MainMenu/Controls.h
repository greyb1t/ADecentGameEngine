#pragma once

#include "MenuComponent.h"

class Controls : public MenuComponent
{
	COMPONENT(Controls, "Controls");
public:
	Controls() = default;
	~Controls() = default;

	void Start() override;

private:

};