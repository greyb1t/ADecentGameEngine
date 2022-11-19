#pragma once

#include "Component.h"
#include "Engine/Renderer/UVRect.h"
#include "Engine/Renderer/VertexTypes.h"
#include "Engine/Renderer/Material/SpriteMaterialInstance.h"

namespace Engine
{
	class SpriteComponent : public Component
	{
	public:
		COMPONENT(SpriteComponent, "SpriteComponent");

		SpriteComponent() = default;
		SpriteComponent(const SpriteComponent& aOther) = default;

		SpriteComponent(
			GameObject* aGameObject,
			const std::string& aSpritePath,
			const int aSortOrder);

		SpriteComponent(GameObject* aGameObject);

		virtual ~SpriteComponent();

		void SetSprite(const std::string& aSpritePath);
		void SetSprite(const TextureRef& aTexture);

		const Vec2ui& GetSize();
		Vec2ui GetSizeWithinUVRect();

		void Execute(eEngineOrder aOrder) override;
		void Render() override;

		void SetMaterial(const std::string& aMaterialName);
		void SetMaterial(const MaterialRef& aMaterial);

		SpriteMaterialInstance& GetMaterialInstance();

		void SetUVRect(const UVRect& aUVRect);

		void SetColor(const C::Vector4f aColor);
		const C::Vector4f& GetColor() const;

		void SetAlpha(const float aAlpha);
		float GetAlpha() const;

		void Reflect(Reflector& aReflector) override;

		const UVRect& GetUV() const;

		SpriteVertex GetSpriteVertex() const;

		TextureRef& GetTexture();

		void SetSortOrder(const int aSortOrder);
		int GetSortOrder() const;

		bool GetIs3D() const;
		void SetIs3D(const bool aIs3D);

		void ClearSprite();

	protected:
		TextureRef myTexture;

		SpriteMaterialInstance myMaterialInstance;

		UVRect myUVRect;

		C::Vector4f myColor = { 1.0f, 1.0f, 1.0f, 1.0f };

		int mySortOrder = 0;

		Vec2f myPivot{0.5f, 0.5f};
		Vec2f myOffset{0.f, 0.f};

		bool myIs3D = false;

		//Renderer::SpriteInstance mySpriteInstance;
	};
}