#pragma once

#include "EditorWindow.h"
#include "Engine\ResourceManagement\ResourceRef.h"

namespace Engine
{
	class DecalMaterial;
}

namespace Engine
{
	class MaterialWindow : public EditorWindow
	{
	public:
		MaterialWindow(Editor& aEditor);

		void OpenMaterial(const MaterialRef& aMaterialResource);

		std::string GetName() const override;

	protected:
		void Draw(const float aDeltaTime) override;
		void DrawDecalMaterial(const float aDeltaTime, DecalMaterial& aDecalMaterial);

		void Save() override;

	private:
		MaterialRef myMaterialResource;

		static inline float ourAlignPercent = 0.3f;
	};
}