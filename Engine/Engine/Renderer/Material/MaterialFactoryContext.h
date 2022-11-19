#pragma once

struct ID3D11Device;


namespace Engine
{
	class ResourceManager;
	class RenderManager;

	// Basically just used to collect the variables needed into one struct
	struct MaterialFactoryContext
	{
		ResourceManager* myResourceManager = nullptr;
		ID3D11Device* myDevice = nullptr;
		// RenderManager* myRenderManager = nullptr;
	};
}