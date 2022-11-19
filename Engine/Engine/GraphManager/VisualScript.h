#pragma once

#include "rapidjson/document.h"

class GraphInstance;
class GameObject;

namespace Engine
{
// currently only holds the already read values from a file itself
// TODO: Cache the values in variables instead, better performance
// even better, use flatbuffers!
class VisualScript
{
public:
	VisualScript();
	~VisualScript();

	bool Init(const Path& aPath);

	GraphInstance* CreateInstance(GameObject* aGameObject);

	// TODO: Make it return by value, because we need to be able to copy it
	// due to it being in a component
	//GraphInstance* CreateInstance();

private:
	rapidjson::Document myDocument;
};
}
