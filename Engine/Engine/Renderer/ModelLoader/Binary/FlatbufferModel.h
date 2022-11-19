#pragma once

namespace Engine
{
	class LoaderModel;
	class LoaderModelBinary;
}

namespace FlatbufferModel
{
	void ConvertAndSaveFlatbufferModel(
		const Engine::LoaderModel& aLoaderModel,
		const Path& aPath);
}
