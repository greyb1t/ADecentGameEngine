#pragma once

#include "ResourceBase.h"
#include "Engine/Reflection/Reflectable.h"

namespace Engine
{
	class MaterialResource;
	class TextureResource;
	class VertexShaderResource;
	class PixelShaderResource;
	class GeometryShaderResource;
	class ModelResource;
	class AnimationClipResource;
	class AnimationStateMachineResource;
	class ResourceBase;
	class VisualScriptResource;
	class AnimationCurveResource;
	class GameObjectPrefabResource;
	class VFXResource;

	// T = is e.g. SpriteResource
	template <typename T>
	class ResourceRef
	{
	public:
		ResourceRef();

		ResourceRef(Shared<T> aResource);
		ResourceRef(const ResourceRef<T>& aOther);

		template <typename U>
		ResourceRef(const ResourceRef<U>& aOther)
			: myResource(aOther.myResource)
		{
		}

		ResourceRef<T>& operator=(const ResourceRef<T>& aOther)
		{
			if (myResource != aOther.myResource)
			{
				myResource = aOther.myResource;

				// We add ref FIRST, VERY IMPORNTANT
				if (aOther.myResource)
				{
					reinterpret_cast<ResourceBase*>(aOther.myResource.get())->AddRef();
				}
			}

			return *this;

			//// We add ref FIRST, VERY IMPORNTANT
			//if (aOther.myResource)
			//{
			//	reinterpret_cast<ResourceBase*>(aOther.myResource.get())->AddRef();
			//}

			//if (myResource)
			//{
			//	reinterpret_cast<ResourceBase*>(myResource.get())->RemoveRef();
			//}

			//myResource = aOther.myResource;

			//return *this;
		}

		~ResourceRef();

		ResourceRef(ResourceRef<T>&& aOther) { myResource = std::move(aOther.myResource); }

		ResourceRef<T>& operator=(ResourceRef<T>&& aOther)
		{
			myResource = std::move(aOther.myResource);
			return *this;
		}

		T* operator->() { return myResource.get(); }

		const T* operator->() const { return myResource.get(); }

		const T& operator*() const { return *myResource; }

		T& operator*() { return *myResource; }

		operator bool() const { return myResource != nullptr; }

		bool operator<(const ResourceRef<T>& aOther) const { return myResource < aOther.myResource; }

		bool operator==(const ResourceRef<T>& aOther) const { return myResource == aOther.myResource; }

		bool operator!=(const ResourceRef<T>& aOther) const { return myResource != aOther.myResource; }

		template <typename U>
		ResourceRef<U> Cast() const;

		Shared<T> myResource = nullptr;
	private:
		template <typename>
		friend class ResourceRef;

	};

	template <typename T>
	template <typename U>
	ResourceRef<U> ResourceRef<T>::Cast() const
	{
		return ResourceRef<U>(std::dynamic_pointer_cast<U>(myResource));
	}

	template <typename T>
	ResourceRef<T>::ResourceRef()
	{
		if (myResource)
		{
			reinterpret_cast<ResourceBase*>(myResource.get())->AddRef();
		}
	}

	template <typename T>
	ResourceRef<T>::ResourceRef(Shared<T> aResource)
		: myResource(aResource)
	{
		if (myResource)
		{
			reinterpret_cast<ResourceBase*>(myResource.get())->AddRef();
		}
	}

	template <typename T>
	ResourceRef<T>::ResourceRef(const ResourceRef<T>& aOther)
	{
		myResource = aOther.myResource;

		if (myResource)
		{
			reinterpret_cast<ResourceBase*>(myResource.get())->AddRef();
		}
	}

	template <typename T>
	ResourceRef<T>::~ResourceRef()
	{
		if (myResource)
		{
			reinterpret_cast<ResourceBase*>(myResource.get())->RemoveRef();
		}
	}
}

namespace std {

	template <typename T>
	struct hash<Engine::ResourceRef<T>>
	{
		std::size_t operator()(const Engine::ResourceRef<T>& aKey) const
		{
			return std::hash<T*>{}(aKey.myResource.get());
		}
	};
}

using MaterialRef = Engine::ResourceRef<Engine::MaterialResource>;
using TextureRef = Engine::ResourceRef<Engine::TextureResource>;
using VertexShaderRef = Engine::ResourceRef<Engine::VertexShaderResource>;
using PixelShaderRef = Engine::ResourceRef<Engine::PixelShaderResource>;
using GeometryShaderRef = Engine::ResourceRef<Engine::GeometryShaderResource>;
using ModelRef = Engine::ResourceRef<Engine::ModelResource>;
using AnimationClipRef = Engine::ResourceRef<Engine::AnimationClipResource>;
using AnimationMachineRef = Engine::ResourceRef<Engine::AnimationStateMachineResource>;
using VisualScriptRef = Engine::ResourceRef<Engine::VisualScriptResource>;
using AnimationCurveRef = Engine::ResourceRef<Engine::AnimationCurveResource>;
using GameObjectPrefabRef = Engine::ResourceRef<Engine::GameObjectPrefabResource>;
using VFXRef = Engine::ResourceRef<Engine::VFXResource>;