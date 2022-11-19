#pragma once

namespace Engine
{
	class Displacement
	{
	public:
		Displacement() = default;
		Displacement(const Vec3f& aPosition, const Vec3f& aRotationEuler);

		void InitFromJson(const nlohmann::json& aJson);

		const Vec3f& GetPosition() const;
		const Vec3f& GetRotationEuler() const;

		static Displacement Lerp(const Displacement& aA, const Displacement& aB, const float aT);
		static Displacement InsideUnitSphere();
		static Displacement Scale(const Displacement& aA, const Displacement& aB);

		Displacement Normalized() const;
		Displacement ScaledBy(const float aPositionScale, const float aRotationScale) const;

		Displacement operator+(const Displacement& aOther) const;
		// Displacement operator*(const float aValue) const;

	private:
		friend Displacement operator*(const Displacement& aLeft, const float aValue);
		friend Displacement operator*(const float aValue, const Displacement& aRight);

		Vec3f myPosition;
		Vec3f myRotationEuler;
	};

	Displacement operator*(const Displacement& aLeft, const float aValue);
	Displacement operator*(const float aValue, const Displacement& aRight);
}