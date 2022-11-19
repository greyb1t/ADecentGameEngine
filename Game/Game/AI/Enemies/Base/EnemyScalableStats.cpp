#include "pch.h"
#include "EnemyScalableStats.h"

#include "Engine/GameObject/GameObject.h"
#include "Engine/Scene/FolderScene.h"
#include "Engine/Reflection/Reflector.h"


EnemyScalableStats::EnemyScalableStats(GameObject* aGameObject) :
	Component(aGameObject)
{
}

void EnemyScalableStats::Start()
{
	Component::Start();
}

void EnemyScalableStats::Reflect(Engine::Reflector& aReflector)
{
	Component::Reflect(aReflector);
	aReflector.Reflect(myEnemyScalableValues.myHealth, "Max Health");
	aReflector.Reflect(myEnemyScalableValues.myDamage, "Damage");
}

void EnemyScalableStats::Execute(Engine::eEngineOrder aOrder)
{
	Component::Execute(aOrder);
}

void EnemyScalableStats::Render()
{
	Component::Render();
}

EnemyScalableValues& EnemyScalableStats::GetScalableValues()
{
	return myEnemyScalableValues;
}

void EnemyScalableStats::SetEnemyScalableValues(EnemyScalableValues aEnemyScalableValues)
{
	myEnemyScalableValues = aEnemyScalableValues;
}

void EnemyScalableStats::SetStatMultiplier(float aStatMultiplier)
{
	myStatMultiplier = aStatMultiplier;
}
