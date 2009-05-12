#ifndef traktor_drawing_PerlinNoiseFilter_H
#define traktor_drawing_PerlinNoiseFilter_H

#include "Drawing/ImageFilter.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DRAWING_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
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
class T_DLLCLASS PerlinNoiseFilter : public ImageFilter
{
	T_RTTI_CLASS(PerlinNoiseFilter)

public:
	PerlinNoiseFilter(int octaves, float persistence, float magnify);

protected:
	virtual Image* apply(const Image* image);

private:
	int m_octaves;
	float m_persistence;
	float m_magnify;
};

	}
}

#endif	// traktor_drawing_PerlinNoiseFilter_H
