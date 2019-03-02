#pragma once

#include "Editor/IThumbnailGenerator.h"

namespace traktor
{
	namespace editor
	{

class ThumbnailGenerator : public IThumbnailGenerator
{
	T_RTTI_CLASS;

public:
	ThumbnailGenerator(const Path& thumbsPath);

	virtual Ref< drawing::Image > get(const Path& fileName, int32_t width, int32_t height, AlphaMode alphaMode) override final;

private:
	Path m_thumbsPath;
};

	}
}

