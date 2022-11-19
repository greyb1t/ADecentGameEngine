#pragma once

namespace Engine
{
	enum eEngineOrder
	{
		EARLY_UPDATE = (1 << 0),
		UPDATE = (1 << 1),

		EARLY_PHYSICS = (1 << 2),
		LATE_PHYSICS = (1 << 3),

		LATE_UPDATE = (1 << 4),

		EARLY_RENDER = (1 << 5),
		LATE_RENDER = (1 << 6),

		FIXED_UPDATE = (1 << 7),
};
}