#pragma once

#include <set>

#include "ResourceManagement\ResourceRef.h"

namespace Engine
{
	// Keeps references to resources to ensure they are not destroyed
	// used for when preloading stuff for the scenes
	// and global resources that are never unloaded
	class ResourceReferences
	{
	public:
		bool AddReferencesFromJson(const Path& aPath);
		bool SaveToJson(const Path& aPath) const;

		TextureRef AddTexture(const Path& aPath);
		VisualScriptRef AddGraph(const Path& aPath);
		ModelRef AddModel(const Path& aPath);
		AnimationClipRef AddAnimationClip(const Path& aPath);
		AnimationMachineRef AddAnimation(const Path& aPath);
		MaterialRef AddMeshMaterial(const Path& aPath);
		PixelShaderRef AddPixelShader(const Path& aPath);
		AnimationCurveRef AddAnimationCurve(const Path& aPath);
		GameObjectPrefabRef AddGameObjectPrefab(const Path& aPath);
		VFXRef AddVFX(const Path& aPath);
		void AddVertexShader(const VertexShaderRef& aResource);

		// If do not know the type of resources, it handles this
		void AddGeneralResource(const ResourceRef<ResourceBase>& aResource);

		void UnloadAll();
		void RequestAll();
		void LoadAll();
		bool AreAllFinishedLoading() const;
		float GetPercentageLoaded() const;

		const std::set<ResourceRef<ResourceBase>>& GetResources() const;

		template <typename T>
		nlohmann::json ResourceVectorToJson(const std::set<ResourceRef<T>>& aResources, const ResourceType aType) const
		{
			nlohmann::json result = nlohmann::json::array();

			// to avoid duplicates
			std::set<ResourceRef<T>> resourcesSet(aResources.begin(), aResources.end());

			for (auto& r : resourcesSet)
			{
				if (r->GetType() != aType)
				{
					continue;
				}

				if (r->IsValid())
				{
					result.push_back(r->GetPath());
				}
			}

			return result;
		}

		template <typename T>
		void LoadResourceVector(const std::set<ResourceRef<T>>& aResources)
		{
			for (auto& r : aResources)
			{
				r->Load();
			}
		}

		template <typename T>
		void RequestResourceVector(const std::set<ResourceRef<T>>& aResources)
		{
			for (auto& r : aResources)
			{
				r->RequestLoading();
			}
		}

		//template <typename T>
		//void UnloadResourceVector(std::set<ResourceRef<T>>& aResources)
		//{
		//	for (auto& r : aResources)
		//	{
		//		r->Unload();
		//	}
		//}

		template <typename T>
		bool IsResourceVectorFinishedLoading(const std::set<ResourceRef<T>>& aResources) const
		{
			for (auto& r : aResources)
			{
				// if (r->GetState() != ResourceState::Loaded)
				// {
				// 	return false;
				// }

				if (r)
				{
					// checks the children as internally as well
					if (!r->IsLoaded())
					{
						return false;
					}
				}
			}

			return true;
		}

		template <typename T>
		float GetPercentageLoaded(const std::set<ResourceRef<T>>& aResources) const
		{
			int totalLoaded = 0;

			for (auto& r : aResources)
			{
				if (r)
				{
					// checks the children as internally as well
					if (r->IsLoaded())
					{
						totalLoaded++;
					}
				}
			}

			return static_cast<float>(totalLoaded) / static_cast<float>(aResources.size());
		}

	private:
		// std::vector<ResourceRef<ResourceBase>>& GetResources(const ResourceType aType);

		//std::vector<ResourceRef<ResourceBase>> myResources;
		std::set<ResourceRef<ResourceBase>> myResources;

		// std::array<
		// 	std::vector<ResourceRef<ResourceBase>>,
		// 	static_cast<int>(ResourceType::Count)> myResources;

		/*
		std::vector<ResourceRef<TextureResource>> myTextures;
		std::vector<ResourceRef<VisualScriptResource>> myGraphs;
		std::vector<ResourceRef<ModelResource>> myModels;
		std::vector<ResourceRef<AnimationClipResource>> myAnimationClips;
		std::vector<ResourceRef<AnimationStateMachineResource>> myAnimationStateMachines;
		std::vector<ResourceRef<MaterialResource>> myMeshMaterials;
		std::vector<ResourceRef<PixelShaderResource>> myPixelShaders;
		std::vector<ResourceRef<VertexShaderResource>> myVertexShaders;
		std::vector<ResourceRef<AnimationCurveResource>> myAnimationCurves;
		std::vector<ResourceRef<GameObjectPrefabResource>> myGameObjectPrefabs;
		std::vector<ResourceRef<VFXResource>> myVFXs;
		*/
	};
}