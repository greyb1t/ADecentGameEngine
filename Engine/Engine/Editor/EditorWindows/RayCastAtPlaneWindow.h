#pragma once

#include "EditorWindow.h"

namespace Engine
{
	class RayCastAtPlaneWindow : public EditorWindow
	{
	public:
		RayCastAtPlaneWindow(Editor& aEditor);

		std::string GetName() const override;

	protected:
		void Draw(const float aDeltaTime) override;

		void Save() override;
		//Vec3f GetRayInWorldSpaceFromMouse();

	private:
		float myPlaneYValue = 0.f;
		Vec3f myResult;
	};
}