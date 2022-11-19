#pragma once
class VFXEditorPropertyWindow
{
public:
	void Tick();

	void Set(const std::function<void()>& aPropertyDrawFunction);
	void Clear();

	static VFXEditorPropertyWindow& Instance();
private:
	std::function<void()> myPropertyDrawFunction;
};

