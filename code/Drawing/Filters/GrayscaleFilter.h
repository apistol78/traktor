#ifndef traktor_drawing_GrayScaleFilter_H
#define traktor_drawing_GrayScaleFilter_H

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
	
/*! \brief Gray scale filter.
 * \ingroup Drawing
 */
class T_DLLCLASS GrayscaleFilter : public ImageFilter
{
	T_RTTI_CLASS(GrayscaleFilter)

protected:
	virtual Ref< Image > apply(const Image* image);
};
	
	}
}

#endif	// traktor_drawing_GrayScaleFilter_H
