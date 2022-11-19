#include "pch.h"
#include "Displacement.h"

Engine::Displacement::Displacement(const Vec3f& aPosition, const Vec3f& aRotationEuler)
	: myPosition(aPosition),
	myRotationEuler(aRotationEuler)
{
}

void Engine::Displacement::InitFromJson(const nlohmann::json& aJson)
{
	myPosition.x = aJson["Position"][0];
	myPosition.y = aJson["Position"][1];
	myPosition.z = aJson["Position"][2];

	myRotationEuler.x = aJson["Rotation"][0];
	myRotationEuler.y = aJson["Rotation"][1];
	myRotationEuler.z = aJson["Rotation"][2];
}

const Vec3f& Engine::Displacement::GetPosition() const
{
	return myPosition;
}

const Vec3f& Engine::Displacement::GetRotationEuler() const
{
	return myRotationEuler;
}

Engine::Displacement Engine::Displacement::Lerp(
	const Displacement& aA,
	const Displacement& aB,
	const float aT)
{
	return Displacement(
		Math::Lerp(aA.GetPosition(), aB.GetPosition(), aT),
		Math::Lerp(aA.GetRotationEuler(), aB.GetRotationEuler(), aT));
}

Engine::Displacement Engine::Displacement::InsideUnitSphere()
{
	return Displacement(Random::InsideUnitSphere(), Random::InsideUnitSphere());
}

Engine::Displacement Engine::Displacement::Scale(const Displacement& aA, const Displacement& aB)
{
	return Displacement(
		Vec3f(
			aA.myPosition.x * aB.myPosition.x,
			aA.myPosition.y * aB.myPosition.y,
			aA.myPosition.z * aB.myPosition.z),
		Vec3f(
			aA.myRotationEuler.x * aB.myRotationEuler.x,
			aA.myRotationEuler.y * aB.myRotationEuler.y,
			aA.myRotationEuler.z * aB.myRotationEuler.z));
}

Engine::Displacement Engine::Displacement::Normalized() const
{
	return Displacement(myPosition.GetNormalized(), myRotationEuler.GetNormalized());
}

Engine::Displacement Engine::Displacement::ScaledBy(const float aPositionScale, const float aRotationScale) const
{
	return Displacement(
		myPosition * aPositionScale,
		myRotationEuler * aRotationScale);
}

Engine::Displacement Engine::Displacement::operator+(const Displacement& aOther) const
{
	return Displacement(
		myPosition + aOther.myPosition,
		myRotationEuler + aOther.myRotationEuler);
}

Engine::Displacement Engine::operator*(const Displacement& aLeft, const float aValue)
{
	return Displacement(
		aLeft.myPosition * aValue,
		aLeft.myRotationEuler * aValue);
}

Engine::Displacement Engine::operator*(const float aValue, const Displacement& aRight)
{
	return Displacement(
		aValue * aRight.myPosition,
		aValue * aRight.myRotationEuler);
}
