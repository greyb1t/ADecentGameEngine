#pragma once
class CameraRig
{
public:
	struct CameraMetrics
	{
		Vec3f myPivotOffset = {0.0f, 0.0f, 0.0f};

		float myUpperCirlceRadius = 0.0f;
		float myUpperCircleHeight = 0.0f;

		float myMiddleCirlceRadius = 0.0f;
		float myMiddleCircleHeight = 0.0f;

		float myLowerCirlceRadius = 0.0f;
		float myLowerCircleHeight = 0.0f;
	};

	CameraRig();
	~CameraRig();

	void LoadMetrics(const CameraMetrics& someMetrics);

	const Vec3f GetPositionOnRig(float aYaw, float aPitch, Vec3f aPlayerPosition) const;
	const Vec3f GetPivotPosition(Vec3f aPlayerPosition) const;

	void DrawRig(Vec3f aPlayerPosition);

private:
	const Vec3f GetPointOnCircle(float aRadius, Vec3f aCirclePos, float aRotation) const;

	CameraMetrics myMetrics;
};
