#pragma once

#include "Engine/Reflection/Reflectable.h"
#include "Engine/ResourceManagement/ResourceRef.h"

namespace Engine
{
	class Material;

	class MaterialInstance
	{
	public:
		virtual ~MaterialInstance() = default;

		bool Init(const MaterialRef& aMeshMaterial);

		void SetFloat4(const std::string& aVariableName, const C::Vector4f& aValue);
		const C::Vector4f& GetFloat4(const std::string& aVariableName);

		// Used in Renderers
		// Modifies the material constant buffer data
		void UpdateMaterialCustomValues();
		void ResetCustomValuesToDefault();

		void Reflect(Reflector& aReflector, const int aMeshIndex);

		bool IsValid() const;

		Material* GetMaterial();
		const MaterialRef& GetMaterialRef() const;

		virtual void ReflectMaterial(Reflector& aReflector) = 0;

		bool AreValuesModifiedFromDefault() const;

		void SetAlpha(const float anAlpha);
		float GetAlpha() const;

		void SetMainColor(const Vec4f& aColor);
		const Vec4f& GetMainColor() const;

		void SetAdditiveColor(const Vec3f& aColor);
		const Vec3f& GetAdditiveColor() const;
		void SetAdditiveIntensity(const float aIntensity);
		float GetAdditiveIntensity() const;

		// Adds the intensity onto its final color
		Vec4f GetFinalAdditiveColor() const;

	protected:
		void SetDefaultValues(Material& aMaterial);

	protected:
		MaterialRef myMeshMaterial;

		// TODO: Currently hardcoded support for Float4 only, change this when needed
		std::unordered_map<std::string, C::Vector4f> myFloats;
		bool myAreValuesModifiedFromDefault = false;

		Vec4f myMainColor = Vec4f(1.f, 1.f, 1.f, 1.f);
		Vec3f myAdditiveColor = Vec3f(0.f, 0.f, 0.f);

	private:
		float myAdditiveIntensity = 1.f;
	};
}
