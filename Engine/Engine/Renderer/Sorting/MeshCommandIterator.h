#pragma once

#include "RenderCommandList.h"

namespace Engine
{
	class MeshCommandIterator
	{
	public:
		MeshCommandIterator(const RenderCommandList& aCommandList);

		bool NotFinished() const;
		void Next();

		const MeshCommandEntry& Get() const { return *myValue; }

		bool EffectPassChanged() const;
		bool ModelChanged() const;
		bool MeshIndexChanged() const;

	private:
		void Update();

	private:
		const std::vector<MeshCommandEntry>& myMeshCommands;
		size_t mySize = 0;
		size_t myIndex = 0;

		const MeshCommandEntry* myValue = nullptr;

		// EffectPass* myBoundPass = nullptr;
		uint16_t myBoundPassID = -1;
		uint16_t myBoundModelID = -1;
		uint16_t myMeshIndex = -1;

		bool myEffectPassChanged = true;
		bool myModelChanged = true;
		bool myMeshIndexChanged = true;
	};
}