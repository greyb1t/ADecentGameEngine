#include "pch.h"
#include "SpriteComponent.h"
#include "..\Engine.h"
#include "..\GameObject.h"
#include "Engine/Reflection/Reflector.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Renderer/GraphicsEngine.h"
#include "Engine/Renderer/Scene/RendererScene.h"
#include "Engine\Renderer\Material\MaterialFactory.h"
#include "Engine/Renderer\Material\MeshMaterial.h"
#include "Engine/ResourceManagement\ResourceManager.h"
#include "Engine/ResourceManagement\Resources\TextureResource.h"
#include "Engine\Renderer\Texture\Texture2D.h"

Engine::SpriteComponent::SpriteComponent(
	GameObject* aGameObject,
	const std::string& aSpritePath,
	const int aSortOrder)
	: Component(aGameObject)
{
	// LOG_INFO(LogType::Filip) << aSpritePath;

	TextureRef textureResource =
		GetEngine().GetResourceManager().CreateRef<TextureResource>(aSpritePath);

	assert(textureResource->GetState() == ResourceState::Loaded);
	// TODO: To be removed, lol, actually all of this should BE GONE
	// TODO: Remove in SetSprite() as well below
	// textureResource->Load();

	//mySpriteInstance.Start(textureResource);

	myTexture = textureResource;

	// TODO: can optimize by getting default material immediately instead of searching for it everytime
	// Set default material
	SetMaterial("");

	mySortOrder = aSortOrder;

	// myImageSize = mySpriteInstance.GetTexture()->Get().GetImageSize();
}

Engine::SpriteComponent::SpriteComponent(GameObject* aGameObject)
	: Component(aGameObject)
{
	// Set default material
	SetMaterial("");
}

Engine::SpriteComponent::~SpriteComponent()
{
}

void Engine::SpriteComponent::SetSprite(const std::string& aSpritePath)
{
	TextureRef textureResource =
		GetEngine().GetResourceManager().CreateRef<TextureResource>(aSpritePath);

	assert(textureResource->GetState() == ResourceState::Loaded);
	// TODO: To be removed, lol, actually all of this should BE GONE
	// textureResource->Load();

	myTexture = textureResource;

	// TODO: can optimize by getting default material immediately instead of searching for it everytime
	// Set default material
	SetMaterial("");

	// myImageSize = mySpriteInstance.GetSprite()->GetData().myTexture->Get().GetImageSize();
}

void Engine::SpriteComponent::SetSprite(const TextureRef& aTexture)
{
	myTexture = aTexture;
}

void Engine::SpriteComponent::Execute(eEngineOrder aOrder)
{
}

void Engine::SpriteComponent::Render()
{

	const auto& transform = myGameObject->GetTransform();

	// mySpriteInstance.GetTransform().SetPosition(transform.GetPositionWorld());
	// // TODO: Fix sprite rotation with new way of doing things, like in ModelInstance with a raw matrix?
	// // sprite.SpriteInstance->GetTransform().SetRotation(transform.Rotation);
	// mySpriteInstance.GetTransform().SetRotationQuaternion(transform.GetRotationWorld());
	// mySpriteInstance.GetTransform().SetScale(transform.GetScaleWorld());

	if (myTexture && myTexture->IsValid() && myMaterialInstance.IsValid())
	{
		//myGameObject->GetScene()->GetUIRendererScene().RenderSprite(*this);
		myGameObject->GetScene()->GetRendererScene().RenderSprite(*this);
	}
}

void Engine::SpriteComponent::SetMaterial(const std::string& aMaterialName)
{
	auto matRef = GetEngine().GetGraphicsEngine().GetMaterialFactory().GetSpriteMaterial(aMaterialName);

	SetMaterial(matRef);
}

void Engine::SpriteComponent::SetMaterial(const MaterialRef& aMaterial)
{
	myMaterialInstance.Init(aMaterial);
}

Engine::SpriteMaterialInstance& Engine::SpriteComponent::GetMaterialInstance()
{
	return myMaterialInstance;
}

void Engine::SpriteComponent::SetUVRect(const UVRect& aUVRect)
{
	myUVRect = aUVRect;
}

void Engine::SpriteComponent::SetColor(const C::Vector4f aColor)
{
	myColor = aColor;
}

const C::Vector4f& Engine::SpriteComponent::GetColor() const
{
	return myColor;
}

void Engine::SpriteComponent::SetAlpha(const float aAlpha)
{
	auto color = GetColor();

	color.w = std::max(aAlpha, 0.f);

	return SetColor(color);
}

float Engine::SpriteComponent::GetAlpha() const
{
	return GetColor().w;
}

void Engine::SpriteComponent::Reflect(Reflector& aReflector)
{
	Component::Reflect(aReflector);

	if (aReflector.Reflect(myTexture, "Texture") & ReflectorResult_Changed)
	{
		SetMaterial("DefaultSprite");
	}

	aReflector.ReflectLambda([&] 
		{
			ImGui::Image(myTexture ? myTexture->Get().GetSRV() : nullptr, {100.f, 100.f});
		});

	aReflector.Reflect(myColor, "Color", ReflectionFlags_IsColor);

	SetColor(myColor);

	aReflector.Reflect(mySortOrder, "Sort Order");
	aReflector.Reflect(myOffset, "Offset");
	aReflector.Reflect(myPivot, "Pivot");

	aReflector.Reflect(myUVRect.myStart, "Start UV");
	aReflector.Reflect(myUVRect.myEnd, "End UV");

	aReflector.Reflect(myIs3D, "Is 3D");

	/*
	if (myGameObject)
	{
		if (myTexture)
		{
			if (!!GDebugDrawer)
			{
				auto& transform = myGameObject->GetTransform();

				const Vec2f pos = { transform.GetPosition().x, transform.GetPosition().y };
				const Vec2f scale = { transform.GetScale().x, transform.GetScale().y };
				Vec2f size = myTexture->Get().GetImageSize().CastTo<float>();
				size = size / GetEngine().GetGraphicsEngine().GetWindowHandler().GetTargetSize().CastTo<float>();
				const Vec2f half_size = size * .5f;

				Vec2f half_extends = myUVRect.myEnd - myUVRect.myStart;

				const Vec2f top_left = pos - half_size * (scale * half_extends);
				const Vec2f bot_right = pos + half_size * (scale * half_extends);

				const Vec2f top_right = { bot_right.x, top_left.y };
				const Vec2f bot_left = { top_left.x, bot_right.y };

				const Vec4f color = { 1.0f, 0.25f, 0.5f, 1.0f };
				GDebugDrawer->DrawLine2D(DebugDrawFlags::Always, top_left, top_right, 0.0f, color);
				GDebugDrawer->DrawLine2D(DebugDrawFlags::Always, top_right, bot_right, 0.0f, color);
				GDebugDrawer->DrawLine2D(DebugDrawFlags::Always, bot_right, bot_left, 0.0f, color);
				GDebugDrawer->DrawLine2D(DebugDrawFlags::Always, bot_left, top_left, 0.0f, color);
			}
		}
	}
	*/

	myMaterialInstance.Reflect(aReflector, 0);
}

const UVRect& Engine::SpriteComponent::GetUV() const
{
	return myUVRect;
}

SpriteVertex Engine::SpriteComponent::GetSpriteVertex() const
{
	SpriteVertex result;

	Vec2f half_extends = myUVRect.myEnd - myUVRect.myStart;
	auto& pos = myGameObject->GetTransform().GetPosition();
	auto rot = myGameObject->GetTransform().GetRotation().EulerAngles();
	result.myPosition = { pos.x, pos.y, pos.z, 1.0f };

	result.myRotationRadians = { rot.z };
	result.mySize = { 
		myGameObject->GetTransform().GetScale().x * half_extends.x,
		myGameObject->GetTransform().GetScale().y * half_extends.y
	};
	result.myUVRect = myUVRect;
	result.myColor = myColor;
	result.myPivot = myPivot;
	result.myOffset = myOffset;
	//result.myPosition = CU::Vector4f(1.f, 1.f, 0.f, 1.f);

	return result;
}

TextureRef& Engine::SpriteComponent::GetTexture()
{
	return myTexture;
}

void Engine::SpriteComponent::SetSortOrder(const int aSortOrder)
{
	mySortOrder = aSortOrder;
}

int Engine::SpriteComponent::GetSortOrder() const
{
	return mySortOrder;
}

const Vec2ui& Engine::SpriteComponent::GetSize()
{
	return myTexture->Get().GetImageSize();
}

Vec2ui Engine::SpriteComponent::GetSizeWithinUVRect()
{
	const auto& textureSize = GetSize();

	Vec2f uv = myUVRect.myEnd - myUVRect.myStart;

	return (textureSize.CastTo<float>() * uv).CastTo<unsigned>();
}

bool Engine::SpriteComponent::GetIs3D() const
{
	return myIs3D;
}

void Engine::SpriteComponent::SetIs3D(const bool aIs3D)
{
	myIs3D = aIs3D;
}

void Engine::SpriteComponent::ClearSprite()
{
	myTexture = {};
}