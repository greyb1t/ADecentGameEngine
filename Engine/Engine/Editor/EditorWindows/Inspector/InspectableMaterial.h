#pragma once

#include "Inspectable.h"
#include "Engine\ResourceManagement\ResourceRef.h"

namespace Engine
{
	class DecalMaterial;
	class MeshMaterial;
	class Material;
	class EffectPass;
}

namespace Engine
{
	class InspectableMaterial : public Inspectable
	{
	public:
		InspectableMaterial(
			Editor& aEditor, 
			InspectorWindow& aInspectorWindow,
			const MaterialRef& aMaterialResource);

		void Draw() override;

		void Save() override;

		std::string GetInspectingType() const override { return "Material"; }

	private:
		void DrawMaterial(Material& aMaterial);

		void DrawRenderTypeCombo(EffectPass& aPass);

		void DrawPixelShaderInput(EffectPass& aPass);
		void DrawGeometryShaderInput(EffectPass& aPass);

		void DrawVertexShaderInput(Material& aMaterial, EffectPass& aPass);
		void DrawRasterizerStateCombo(EffectPass& aPass);
		void DrawBlendStateCombo(EffectPass& aPass);
		void DrawTextureList(EffectPass& aPass);
		void DrawConstantBufferList(EffectPass& aPass);

		void DrawDecalMaterial(DecalMaterial& aDecalMaterial);
		void DrawMeshMaterial(MeshMaterial& aMeshMaterial);

	private:
		MaterialRef myMaterialResource;

		static inline float ourAlignPercent = 0.3f;
	};
}