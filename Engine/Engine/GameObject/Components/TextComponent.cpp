#include "pch.h"
#include "TextComponent.h"
#include "..\Engine.h"
#include "..\GameObject.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Renderer/GraphicsEngine.h"
#include "Engine/Renderer/Scene/RendererScene.h"
#include "Engine\Renderer\Material\MaterialFactory.h"
#include "Engine/Renderer\Material\MeshMaterial.h"

#include "Engine/Renderer\Text/Text.h"
#include "Engine/Renderer\Text/Font.h"
#include "Engine/Renderer\Text/TextFactory.h"
#include "Engine/Renderer\Text/TextRenderer.h"
#include "Engine/Reflection/Reflector.h"

namespace Engine
{
	TextComponent::TextComponent(
		GameObject* aGameObject,
		const std::string& aText,
		const int aSortOrder)
		: Component(aGameObject)
	{
		myString = aText;
		myText = Engine::GetInstance().GetTextFactory().CreateText(aText, myWorldSize, myIsCentered);
	}

	TextComponent::~TextComponent()
	{
	}

	void TextComponent::OnConstruct()
	{
		myText = Engine::GetInstance().GetTextFactory().CreateText(myString, myWorldSize, myIsCentered);
	}

	void TextComponent::Execute(eEngineOrder aOrder)
	{
	}

	void TextComponent::Render()
	{
		const auto& transform = myGameObject->GetTransform();

		myText.GetTransform().SetPosition(transform.GetPosition());
		myText.GetTransform().SetRotationQuaternion(transform.GetRotation());
		myText.GetTransform().SetScale(transform.GetScale());
		myText.SetColor(myColor);

		// Quick fix to account for text being really large in screenspace.
		if (myIsScreenSpace)
		{
			myText.GetTransform().SetScale(transform.GetScale() * 0.001f);
			myText.SetSortOrder(mySortOrder);
		}

		myText.GetIs2D() = myIsScreenSpace;

		myGameObject->GetScene()->GetRendererScene().RenderText(myText);
	}

	const C::Vector4f& TextComponent::GetColor() const
	{
		return myColor;
	}

	void TextComponent::SetColor(const C::Vector4f& aColor)
	{
		myColor = aColor;
	}

	void TextComponent::SetAlpha(float aAlpha)
	{
		myColor.w = aAlpha;
	}

	void TextComponent::Reflect(Reflector& aReflector)
	{
		Component::Reflect(aReflector);
		if (aReflector.Reflect(myString, "Text") & ReflectorResult_Changed)
		{
			myText = Engine::GetInstance().GetTextFactory().CreateText(myString, myWorldSize, myIsCentered);
		}
		aReflector.Reflect(mySortOrder, "Sort order");
		aReflector.Reflect(myIsScreenSpace, "Screen space");
		aReflector.Reflect(myColor, "Color");

		if(aReflector.Reflect(myIsCentered, "Is Centered") & ReflectorResult_Changed)
		{
			myText = Engine::GetInstance().GetTextFactory().CreateText(myString, myWorldSize, myIsCentered);
		}

		if(aReflector.Reflect(myWorldSize, "Font size") & ReflectorResult_Changed)
		{
			myText = Engine::GetInstance().GetTextFactory().CreateText(myString, myWorldSize, myIsCentered);
		}

		//aReflector.ReflectLambda([&] 
		//	{
		//		ImGui::Text("TEXT: ");
		//		ImGui::SameLine(); 
		//		ImGui::Text(myText.GetText().c_str()); 
		//	});
	}

	const Vec3f& TextComponent::GetPosition() const
	{
		return myText.GetTransform().GetPosition();

	}

	void TextComponent::SetText(const std::string& aText)
	{
		myString = aText;
		myText = Engine::GetInstance().GetTextFactory().CreateText(myString, myWorldSize, myIsCentered);
	}

	const std::string& TextComponent::GetText() const
	{
		return myString;
	}

	void TextComponent::SetIsCentered(const bool aIsCentered)
	{
		myIsCentered = aIsCentered;
		myText = Engine::GetInstance().GetTextFactory().CreateText(myString, myWorldSize, myIsCentered);
	}

	void TextComponent::SetSortOrder(int aLayer)
	{
		mySortOrder = aLayer;
	}
}