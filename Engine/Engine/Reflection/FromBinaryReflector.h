#pragma once

#include "Reflector.h"

class BinaryReader;
class Component;

namespace Engine
{
	class FromBinaryReflector : public Reflector
	{
	public:
		FromBinaryReflector(BinaryReader& aReader)
			: myReader(aReader)
		{}

		ReflectorResult ReflectInternal(int& aValue, const std::string& aName, const ReflectionFlags aFlags = ReflectionFlags_None) override;

		ReflectorResult ReflectInternal(uint64_t& aValue, const std::string& aName, const ReflectionFlags aFlags = ReflectionFlags_None) override;

		ReflectorResult ReflectInternal(float& aValue, const std::string& aName, const ReflectionFlags aFlags = ReflectionFlags_None) override;

		ReflectorResult ReflectInternal(Vec2f& aValue, const std::string& aName, const ReflectionFlags aFlags = ReflectionFlags_None) override;

		ReflectorResult ReflectInternal(Vec3f& aValue, const std::string& aName, const ReflectionFlags aFlags = ReflectionFlags_None) override;

		ReflectorResult ReflectInternal(Vec4f& aValue, const std::string& aName, const ReflectionFlags aFlags = ReflectionFlags_None) override;

		ReflectorResult ReflectInternal(bool& aValue, const std::string& aName, const ReflectionFlags aFlags = ReflectionFlags_None) override;

		ReflectorResult ReflectInternal(std::string& aValue, const std::string& aName, const ReflectionFlags aFlags = ReflectionFlags_None) override;

		ReflectorResult ReflectInternal(ModelRef& aValue, const std::string& aName, const ReflectionFlags aFlags = ReflectionFlags_None) override;

		ReflectorResult ReflectInternal(MaterialRef& aValue, const MaterialType aType, const std::string& aName, const ReflectionFlags aFlags = ReflectionFlags_None) override;

		ReflectorResult ReflectInternal(AnimationPair& aValue, const std::string& aName, const ReflectionFlags aFlags = ReflectionFlags_None) override;

		ReflectorResult ReflectInternal(TextureRef& aValue, const std::string& aName, const ReflectionFlags aFlags = ReflectionFlags_None) override;

		ReflectorResult ReflectInternal(VisualGraphPair& aValue, const std::string& aName, const ReflectionFlags aFlags = ReflectionFlags_None) override;

		ReflectorResult ReflectInternal(AnimationCurveRef& aValue, const std::string& aName, const ReflectionFlags aFlags = ReflectionFlags_None) override;

		ReflectorResult ReflectInternal(GameObjectPrefabRef& aValue, const std::string& aName, const ReflectionFlags aFlags = ReflectionFlags_None) override;

		ReflectorResult ReflectInternal(VFXRef& aValue, const std::string& aName, const ReflectionFlags aFlags = ReflectionFlags_None) override;

		ReflectorResult ReflectInternal(Transform& aValue, const std::string& aName, const ReflectionFlags aFlags = ReflectionFlags_None) override;

		ReflectorResult ReflectInternal(
			GameObjectRef& aValue,
			const std::string& aName,
			const ReflectionFlags aFlags = ReflectionFlags_None) override;

		ReflectorResult ReflectInternal(Enum& aValue, const std::string& aName, const ReflectionFlags aFlags = ReflectionFlags_None) override;

	private:
		Component* myFrom = nullptr;
		Component* myTo = nullptr;

		BinaryReader& myReader;
	};
}