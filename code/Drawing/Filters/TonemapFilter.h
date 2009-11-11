#ifndef traktor_drawing_TonemapFilter_H
#define traktor_drawing_TonemapFilter_H

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

/*! \brief Tone map filter.
 * \ingroup Drawing
 *
 * Adjust dynamic range of image so
 * it fits the human visual range.
 */
class T_DLLCLASS TonemapFilter : public ImageFilter
{
	T_RTTI_CLASS(TonemapFilter)

protected:
	virtual Ref< Image > apply(const Image* image);
};
	
	}
}

#endif	// traktor_drawing_TonemapFilter_H
