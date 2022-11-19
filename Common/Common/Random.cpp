#include "Random.h"

#include <random>
#include <algorithm>

static std::mt19937 globalRandomEngine{std::random_device{}()};

float Random::RandomFloat(const float aMin, const float aMax)
{
	std::uniform_real_distribution<float> dist(std::min(aMin, aMax), std::max(aMax, aMin));
	return dist(globalRandomEngine);
}

int Random::RandomInt(const int aMin, const int aMax)
{
	std::uniform_int_distribution<int> dist(std::min(aMin, aMax), std::max(aMax, aMin));
	return dist(globalRandomEngine);
}

unsigned int Random::RandomUInt(const unsigned aMin, const unsigned aMax)
{
	std::uniform_int_distribution<unsigned> dist(std::min(aMin, aMax), std::max(aMax, aMin));
	return dist(globalRandomEngine);
}

size_t Random::RandomUInt(const size_t aMin, const size_t aMax)
{
	std::uniform_int_distribution<size_t> dist(std::min(aMin, aMax), std::max(aMax, aMin));
	return dist(globalRandomEngine);
}

CU::Vector3f Random::InsideUnitSphere()
{
	return CU::Vector3f(
		RandomFloat(-1.f, 1.f),
		RandomFloat(-1.f, 1.f),
		RandomFloat(-1.f, 1.f)).GetNormalized();
}
