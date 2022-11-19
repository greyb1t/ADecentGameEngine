#pragma once

#include "../ResourceManager.h"
#include "../ResourceId.h"
#include "../ResourceBase.h"
#include "Engine/Renderer/Shaders/Shader.h"

namespace Engine
{
	class VertexShader;
}

namespace Engine
{
	class VertexShaderResource : public ResourceBase
	{
	public:
		static inline ResourceType ourResourceType = ResourceType::VertexShader;

	public:
		VertexShaderResource(
			ResourceManager& aResourceManager,
			const std::string& aPath,
			const ResourceId& aId,
			const D3D11_INPUT_ELEMENT_DESC* aElements,
			const int aElementCount,
			const std::vector<ShaderDefines>& aDefines = { });

		~VertexShaderResource();

		VertexShaderResource(const VertexShaderResource& aOther);

		VertexShader& Get() const;

	protected:
		friend class ResourceManager;

		bool Create(ResourceThreadData& aThreadData) override;
		void Swap(ResourceBase* aResourceBase);
		void PostCreation(ResourceThreadData& aResourceThreadData) override;
		void RequestMe() const;
		void LoadMe() const;
		void UnloadMe();

	private:
		Owned<VertexShader> myVertexShader = nullptr;

		const D3D11_INPUT_ELEMENT_DESC* myElements = nullptr;
		const int myElementCount = 0;

		std::vector<std::string> myIncludedFilePaths;

		std::vector<ShaderDefines> myDefines;
	};
}