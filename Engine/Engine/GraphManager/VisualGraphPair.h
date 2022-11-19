#pragma once

#include "Engine/ResourceManagement/ResourceRef.h"

class GraphInstance;

namespace Engine
{
	// Purpose is to send this to the reflection system and it chooses
	// what to edit based on if the instance is valid
	struct VisualGraphPair
	{
		// MUST HAVE A REFERENCE LIKE THIS, OTHERWISE THE REFLECT function cannot edit the ResourceRef
		VisualGraphPair(VisualScriptRef& aVisualGraphResource)
			: myVisualGraphResource(aVisualGraphResource)
		{
		}

		bool operator==(const VisualGraphPair& aOther) const
		{
			return myVisualGraphResource == aOther.myVisualGraphResource;
		}

		bool operator!=(const VisualGraphPair& aOther) const
		{
			return !(*this == aOther);
		}

		VisualGraphPair& operator=(const VisualGraphPair& aOther)
		{
			myVisualGraphResource = aOther.myVisualGraphResource;
			return *this;
		}

		VisualScriptRef& myVisualGraphResource;
		GraphInstance* myGraphInstance = nullptr;
	};
}