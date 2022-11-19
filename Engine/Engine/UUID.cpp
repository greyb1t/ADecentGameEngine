#include "pch.h"
#include "UUID.h"

static std::random_device globalRandomDevice;
static std::mt19937_64 globalEngine(globalRandomDevice());
static std::uniform_int_distribution<int> globalUniformDistribution;

Engine::UUID::UUID()
	: myUUID(globalUniformDistribution(globalEngine))
{
}

Engine::UUID::UUID(const int aUUID)
	: myUUID(aUUID)
{
}
