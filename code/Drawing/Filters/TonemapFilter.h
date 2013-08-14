#ifndef traktor_drawing_TonemapFilter_H
#define traktor_drawing_TonemapFilter_H

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

/*! \brief Tone map filter.
 * \ingroup Drawing
 *
 * Adjust dynamic range of image so
 * it fits the human visual range.
 */
class T_DLLCLASS TonemapFilter : public IImageFilter
{
	T_RTTI_CLASS;

protected:
	virtual void apply(Image* image) const;
};
	
	}
}

#endif	// traktor_drawing_TonemapFilter_H
