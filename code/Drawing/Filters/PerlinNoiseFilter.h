#ifndef traktor_drawing_PerlinNoiseFilter_H
#define traktor_drawing_PerlinNoiseFilter_H

#include "Drawing/IImageFilter.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DRAWING_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace drawing
	{

/*! \brief Perlin noise image filter.
 * \ingroup Drawing
 *
 * Create image filled with perlin based noise.
 */
class T_DLLCLASS PerlinNoiseFilter : public IImageFilter
{
	T_RTTI_CLASS;

public:
	PerlinNoiseFilter(int octaves, float persistence, float magnify);

protected:
	virtual void apply(Image* image) const;

private:
	int m_octaves;
	float m_persistence;
	float m_magnify;
};

	}
}

#endif	// traktor_drawing_PerlinNoiseFilter_H
