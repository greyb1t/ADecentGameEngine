#pragma once

namespace FB
{
	class FinalBossWaypoint
	{
	public:
		enum class TakenBy
		{
			LeftHand,
			RightHand,
			None
		};

		FinalBossWaypoint() = default;
		FinalBossWaypoint(const Vec3f& aPosition);

		void Take();
		void Release();
		bool IsTaken() const;

		const Vec3f& GetPosition() const;

	private:
		Vec3f myPosition;
		bool myIsTaken = false;
	};
}