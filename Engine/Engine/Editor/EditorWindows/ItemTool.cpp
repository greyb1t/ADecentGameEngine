#include "pch.h"
#include "ItemTool.h"


namespace Engine
{
	void ItemTool::Update()
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Tools"))
			{
				if (ImGui::MenuItem("Item Editor", "", myShouldDisplay))
				{
					myShouldDisplay = !myShouldDisplay; 
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}
	}

}
