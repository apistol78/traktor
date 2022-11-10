#pragma once

#include "Drawing/IImageFilter.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DRAWING_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::drawing
{

/*! Perlin noise image filter.
 * \ingroup Drawing
 *
 * Create image filled with perlin based noise.
 */
class T_DLLCLASS PerlinNoiseFilter : public IImageFilter
{
	T_RTTI_CLASS;

public:
	explicit PerlinNoiseFilter(int octaves, float persistence, float magnify);

protected:
	virtual void apply(Image* image) const override final;

private:
	int m_octaves;
	float m_persistence;
	float m_magnify;
};

}
