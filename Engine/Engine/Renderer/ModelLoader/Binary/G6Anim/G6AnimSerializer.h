#pragma once

#include "G6Anim.h"

namespace Engine
{
	class G6AnimSerializer
	{
	public:
		G6AnimSerializer();

		bool Serialize(G6Anim aG6Anim);

		const std::vector<uint8_t>& GetBuffer();

	private:
		BinaryWriter myMainWriter;
	};
}