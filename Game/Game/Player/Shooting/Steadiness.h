#pragma once
class Steadiness
{
public:
	Steadiness() = default;
	Steadiness(float aSteadinessRecover);

	void Update();
	void Add(float aValue);
	void Recover(float aValue);

	float Get() const;
private:
	float mySteadiness = 0;
	float mySteadinessRecover = 1;
};

