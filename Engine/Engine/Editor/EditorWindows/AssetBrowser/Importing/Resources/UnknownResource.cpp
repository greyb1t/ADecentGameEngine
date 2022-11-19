#include "pch.h"
#include "UnknownResource.h"

Engine::UnknownResource::UnknownResource(const std::filesystem::path& aPath)
	: Resource(aPath)
{
}

bool Engine::UnknownResource::Draw(ModelImportState& aModelCreationState)
{
	if (ImGui::Button("As my ass"))
	{
		return true;
	}

	return false;
}
