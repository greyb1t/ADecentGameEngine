#pragma once

#include "Component.h"
#include "Engine/Renderer/Text/Text.h"

namespace Engine
{
	class Text;
}

namespace Engine
{
	class TextComponent : public Component
	{
	public:
		COMPONENT(TextComponent, "TextComponent");

		TextComponent() = default;
		TextComponent(const TextComponent& aOther) = default;
		TextComponent(
			GameObject* aGameObject,
			const std::string& aText,
			const int aSortOrder);

		virtual ~TextComponent();

		void OnConstruct() override;

		void Execute(eEngineOrder aOrder) override;
		void Render() override;

		const C::Vector4f& GetColor() const;
		void SetColor(const C::Vector4f& aColor);
		void SetAlpha(float aAlpha);

		void Reflect(Reflector& aReflector) override;

		const Vec3f& GetPosition() const;

		void SetText(const std::string& aText);
		void SetIsInScreenSpace(bool anIsScreenSpace) { myIsScreenSpace = anIsScreenSpace; }
		void SetIsCentered(const bool aIsCentered);
		void SetSortOrder(int aLayer);

		const std::string& GetText() const;

	private:
		Text myText;
		std::string myString = "Default text";
		bool myIsScreenSpace = false;
		int mySortOrder = 0;
		Vec4f myColor = {1, 1, 1, 1};
		bool myIsCentered = false;
		float myWorldSize = 1.f;
	};
}