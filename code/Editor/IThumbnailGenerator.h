#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Io/Path.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::drawing
{

class Image;

}

namespace traktor::editor
{

class T_DLLCLASS IThumbnailGenerator : public Object
{
	T_RTTI_CLASS;

public:
	enum class Alpha
	{
		NoAlpha = 0,
		WithAlpha = 1,
		AlphaOnly = 2
	};

	enum class Gamma
	{
		Auto = 0,
		Linear = 1,
		SRGB = 2
	};

	virtual Ref< drawing::Image > get(const Path& fileName, int32_t width, int32_t height, Alpha alpha, Gamma gamma) = 0;
};

}
