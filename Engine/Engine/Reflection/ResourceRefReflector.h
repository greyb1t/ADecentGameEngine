#pragma once

#include "Reflector.h"
#include "Engine/ResourceReferences.h"

namespace Engine
{
	class ResourceRefReflector : public Reflector
	{
	public:
		ResourceRefReflector();
		ReflectorResult ReflectInternal(
			ModelRef& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags) override;

		ReflectorResult ReflectInternal(
			MaterialRef& aValue,
			const MaterialType aType,
			const std::string& aName,
			const ReflectionFlags aFlags) override;

		ReflectorResult ReflectInternal(
			AnimationPair& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags) override;

		ReflectorResult ReflectInternal(
			TextureRef& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags) override;

		ReflectorResult ReflectInternal(
			VisualGraphPair& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags) override;

		ReflectorResult ReflectInternal(
			AnimationCurveRef& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags) override;

		ReflectorResult ReflectInternal(
			GameObjectPrefabRef& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags) override;

		ReflectorResult ReflectInternal(
			VFXRef& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags) override;

		ResourceReferences& GetResourceRefs();

		const ResourceReferences& GetResourceRefs() const;

	private:
		void AddChildren(const ResourceRef<ResourceBase>& aResource);

	private:
		ResourceReferences myResRefs;
	};
}
