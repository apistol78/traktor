#ifndef traktor_drawing_NormalMapFilter_H
#define traktor_drawing_NormalMapFilter_H

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

/*! \brief Normal map image filter.
 * \ingroup Drawing
 *
 * Calculate a normal map by sampling heights from
 * source image.
 */
class T_DLLCLASS NormalMapFilter : public ImageFilter
{
	T_RTTI_CLASS(NormalMapFilter)

public:
	NormalMapFilter(float scale);

protected:
	virtual Image* apply(const Image* image);
};
	
	}
}

#endif	// traktor_drawing_NormalMapFilter_H
