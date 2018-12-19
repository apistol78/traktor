#pragma once

#include "Core/Object.h"
#include "Core/RefArray.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace cmft
{

struct ClContext;

}

namespace traktor
{
	namespace drawing
	{

class Image;

	}

	namespace render
	{

class CubeMap;

class T_DLLCLASS ProbeProcessor : public Object
{
	T_RTTI_CLASS;

public:
	ProbeProcessor();

	virtual ~ProbeProcessor();

	bool create();

	void destroy();

	bool radiance(const drawing::Image* image, int32_t glossScale, int32_t glossBias, RefArray< CubeMap >& outCubeMips) const;

	bool irradiance(const drawing::Image* image, float factor, int32_t faceSize, RefArray< CubeMap >& outCubeMips) const;

private:
	cmft::ClContext* m_clContext;
};

	}
}
