#include "pch.h"
#include "Temp.h"
#include "Reflection/ToBinaryReflector.h"
#include "Reflection/FromBinaryReflector.h"
#include "GameObject/Components/Component.h"

void Engine::Temp(Component* aFrom, Component* aTo)
{
	ToBinaryReflector toBinaryRefl;

	aFrom->Reflect(toBinaryRefl);

	BinaryReader reader;
	reader.InitFromBinaryWriter(toBinaryRefl.GetWriter());

	FromBinaryReflector fromBinaryRefl(reader);
	aTo->Reflect(fromBinaryRefl);
}
