#pragma once

namespace Engine
{
	class Reflector;

	class Reflectable
	{
	public:
		virtual ~Reflectable() = default;

		virtual void Reflect(Reflector& aReflector) = 0;
	};
}