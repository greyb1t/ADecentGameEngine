#pragma once

namespace Engine
{
#define NUM_BONES_PER_VEREX 4

	struct VertexBoneData
	{
		unsigned int myIDs[NUM_BONES_PER_VEREX] = {};
		float myWeights[NUM_BONES_PER_VEREX] = {};

		void AddBoneData(unsigned int aBoneID, float aWeight)
		{
			for (unsigned int i = 0; i < NUM_BONES_PER_VEREX; i++)
			{
				if (myWeights[i] == 0.0)
				{
					myIDs[i] = aBoneID;
					myWeights[i] = aWeight;
					return;
				}
			}

			// should never get here - more bones than we have space for
			//assert(false);
			LOG_WARNING(LogType::Engine) << "Model had more weights than supported, max 4 weights";
		}
	};
}