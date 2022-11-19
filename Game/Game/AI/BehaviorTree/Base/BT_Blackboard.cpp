#include "pch.h"
#include "BT_Blackboard.h"

GameObject* BT_Blackboard::GetOwner()
{
	return myOwner;
}

void BT_Blackboard::SetAnimationController(Engine::AnimationController* aAnimationController)
{
	myAnimationController = aAnimationController;
}

Engine::AnimationController* BT_Blackboard::GetAnimationController()
{
	return myAnimationController;
}

void BT_Blackboard::SetAudioComponent(Engine::AudioComponent* aAudioComponent)
{
	myAudioComponent = aAudioComponent;
}

Engine::AudioComponent* BT_Blackboard::GetAudioComponent() const
{
	return myAudioComponent;
}

bool BT_Blackboard::getBool(std::string key)
{
	if (bools.find(key) == bools.end()) {
		bools[key] = false;
	}
	return bools[key];
}

int BT_Blackboard::getInt(std::string key)
{
	if (ints.find(key) == ints.end()) {
		ints[key] = 0;
	}
	return ints[key];
}

float& BT_Blackboard::getFloat(std::string key)
{
	if (floats.find(key) == floats.end()) {
		floats[key] = 0.0f;
	}
	return floats[key];
}

double BT_Blackboard::getDouble(std::string key)
{
	if (doubles.find(key) == doubles.end()) {
		doubles[key] = 0.0f;
	}
	return doubles[key];
}

std::string BT_Blackboard::getString(std::string key)
{
	if (strings.find(key) == strings.end()) {
		strings[key] = "";
	}
	return strings[key];
}

Vec3f BT_Blackboard::getVec3f(std::string key)
{
	if (vec3fs.find(key) == vec3fs.end()) {
		vec3fs[key] = Vec3f(0.f, 0.f, 0.f);
	}
	return vec3fs[key];
}

Enemy::eRapidShootData& BT_Blackboard::GetRapidShootData()
{
	return myRapidShootData;
}

Enemy::eGroundPopcornShootData& BT_Blackboard::GetGroundPopcornShootData()
{
	return myGroundPopcornShootData;
}

Enemy::eBurstShootData& BT_Blackboard::GetBurstShootData()
{
	return myBurstShootData;
}

Enemy::eMortarShootData& BT_Blackboard::GetMortarShootData()
{
	return myMortarShootData;
}

void BT_Blackboard::SetGameObject(GameObject* aGameObject)
{
	myOwner = aGameObject;
}
