#pragma once
namespace VFXEditorHelpers
{
	inline bool TitleButton(const std::string& title, const std::string& buttonTitle, float indent, ImU32 backgroundColor = IM_COL32(90, 20, 145, 155))
	{
		ImGui::Separator();
		auto min = ImGui::GetItemRectMin();
		auto max = ImGui::GetItemRectMax();
		min.y += 14.f;
		max.y += 14.f;
		min.x += indent;

		ImGui::GetWindowDrawList()->AddLine(min, max, backgroundColor, 28.f);
		ImGui::Text(title.c_str());
		ImGui::SameLine();

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 3.f);
		return ImGui::Button(buttonTitle.c_str());
	}
	inline bool TitleButton(const std::string& title, const std::string& buttonTitle, ImU32 backgroundColor = IM_COL32(90, 20, 145, 155))
	{
		return TitleButton(title, buttonTitle, 0, backgroundColor);
	}
	inline void TitleBar(const std::string& title, float indent, ImU32 backgroundColor = IM_COL32(128, 0, 128, 155))
	{
		ImGui::Separator();
		auto min = ImGui::GetItemRectMin();
		auto max = ImGui::GetItemRectMax();
		min.y += 14.f;
		max.y += 14.f;
		min.x += indent;

		ImGui::GetWindowDrawList()->AddLine(min, max, backgroundColor, 28.f);
		ImGui::Text(title.c_str());
	}
	inline void TitleBar(const std::string& title, ImU32 backgroundColor = IM_COL32(128, 0, 128, 155))
	{
		TitleBar(title, 0, backgroundColor);
	}

	inline void DrawSpace(eSpace& aSpace)
	{
		const char* spaces[] = { "WorldSpace", "LocalSpace" };
		int spaceIndex = static_cast<int>(aSpace);
		if (ImGui::Combo("Space", &spaceIndex, spaces, IM_ARRAYSIZE(spaces)))
		{
			aSpace = static_cast<eSpace>(spaceIndex);
		}
	}

	inline void DrawTransform(Vec3f& aPos, Quatf& aRot)
	{
		ImGui::Text("Transform");
		ImGui::DragFloat3("Position", &aPos.x);
		Vec3f rot = aRot.EulerAngles() * Math::Rad2Deg;
		if (ImGui::DragFloat3("Rotation", &rot.x))
		{
			aRot = Quatf(rot * Math::Deg2Rad);
		}
	}

	inline void DrawSetting(unsigned& aSettings, unsigned aSettingsAvailable)
	{
		ImGui::BeginGroup();
		const char* settings[] = { "Lerp", "Modifier" };

		for (int i = 0; i < IM_ARRAYSIZE(settings); i++)
		{
			if (!(aSettingsAvailable & (1 << i)))
				continue;

			bool val = aSettings & (1 << i);
			if (ImGui::Checkbox(settings[i], &val))
			{
				aSettings ^= 1UL << i;
			}
		}

		ImGui::EndGroup();
	}

	inline void DrawGeometry(EmitterGeometry& aGeometry)
	{
		ImGui::Text("Geometry");

		const char* shapeTypes[] = { "Point", "Sphere", "Box", "Cone" };
		int shapeIndex = static_cast<int>(aGeometry.GetShape());
		if (ImGui::Combo("Shape", &shapeIndex, shapeTypes, IM_ARRAYSIZE(shapeTypes)))
		{
			switch (static_cast<eEmitShape>(shapeIndex))
			{
			case eEmitShape::POINT: 
				break;
			case eEmitShape::SPHERE:
				aGeometry = EmitterGeometry::Sphere(100, 0);
				break;
			case eEmitShape::BOX: 
				aGeometry = EmitterGeometry::Box({ 50, 50, 50 });
				break;
			case eEmitShape::CONE: 
				aGeometry = EmitterGeometry::Cone(100, 45);
				break;
			default:;
			}
		}

		const char* fromTypes[] = { "Edge", "Volume", "Origin" };
		int fromIndex = static_cast<int>(aGeometry.GetEmitFrom());
		if (ImGui::Combo("Emit From", &fromIndex, fromTypes, IM_ARRAYSIZE(fromTypes)))
		{
			aGeometry.SetFrom(static_cast<eEmitFrom>(fromIndex));
		}

		const char* directionTypes[] = { "Default", "Outwards", "Random" };
		int directionIndex = static_cast<int>(aGeometry.GetEmitDirection());
		if (ImGui::Combo("Emit Direction", &directionIndex, directionTypes, IM_ARRAYSIZE(directionTypes)))
		{
			aGeometry.SetDirection(static_cast<eEmitDirection>(directionIndex));
		}

		switch (aGeometry.GetShape())
		{
		case eEmitShape::POINT:
			{
				
			}
			break;
		case eEmitShape::SPHERE:
			{
			ImGui::DragFloat("Radius", &aGeometry.properties.sphere.radius);
			ImGui::DragFloat("Min Radius", &aGeometry.properties.sphere.minRadius);
			}
			break;
		case eEmitShape::BOX:
			{
			ImGui::DragFloat3("Half Size", &aGeometry.properties.box.halfSize.x);
			}
			break;
		case eEmitShape::CONE:
			{
			ImGui::DragFloat("Radius", &aGeometry.properties.cone.radius);
			ImGui::DragFloat("Angle", &aGeometry.properties.cone.angle);
			}
			break;
		default: ;
		}
	}

	inline void DrawTimeline(Timeline& aTimeline)
	{
		ImGui::Text("Timeline");
		auto& keys = aTimeline.Keys();

		if (ImGui::Button("Add Key"))
		{
			aTimeline.Add(Timeline::Key::Single(0));
			aTimeline.Sort();
		}

		for (int i = 0; i < keys.size(); i++)
		{
			ImGui::PushID(i);

			auto& k = keys[i];
			std::string title = "Key: ";
			if (ImGui::CollapsingHeader(title.c_str()))
			{
				ImGui::SameLine();
				ImGui::Text(std::to_string(k.time).c_str());

				ImGui::SliderFloat("Time", &k.time, 0, 1, "%0.3f", ImGuiSliderFlags_None);
				ImGui::SliderFloat("Time Modifier", &k.timeRandom, 0, 1, "%0.3f", ImGuiSliderFlags_None);

				ImGui::Spacing();
				{
					int amount = k.amount;
					if (ImGui::InputInt("Amount", &amount))
						if (amount >= 0)
							k.amount = amount;
				}
				{
					int amountRandom = k.amountRandom;
					if (ImGui::InputInt("Amount Modifier", &amountRandom))
						if (amountRandom >= 0)
							k.amountRandom = amountRandom;
				}

				ImGui::Spacing();
				ImGui::Checkbox("Looping", &k.isLooping);
				
				if (k.isLooping)
				{
					ImGui::InputFloat("Times Per Second", &k.timesPerSecond, 1, 10, "%.1f", ImGuiSliderFlags_None);
				} else
				{
					int repeatTimes = k.repeatTimes;
					if (ImGui::InputInt("Repeat Times", &repeatTimes))
						if (repeatTimes >= 0)
							k.repeatTimes = repeatTimes;

					if (k.repeatTimes > 0)
					{
						ImGui::SliderFloat("Repeat Delay", &k.repeatDelay, 0, 1, "%0.3f", ImGuiSliderFlags_None);
						ImGui::SliderFloat("Repeat Delay Modifier", &k.repeatDelayRandom, 0, 1, "%0.3f", ImGuiSliderFlags_None);
					}
				}


				ImGui::Spacing();
				if (ImGui::Button("Remove"))
				{
					keys.erase(keys.begin() + i);
					--i;
				}
			} else
			{
				ImGui::SameLine();
				ImGui::Text(std::to_string(k.time).c_str());
			}

			ImGui::PopID();
		}
	}
};

