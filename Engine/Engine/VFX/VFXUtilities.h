#pragma once
#include "Description/VFXDescription.h"

namespace VFX
{
	void Save(const VFXDescription& aDescription);
	VFXDescription Load(const std::string& aFilePath, bool* aSucceeded = nullptr);
};