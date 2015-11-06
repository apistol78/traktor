#ifndef traktor_drawing_NormalizeFilter_H
#define traktor_drawing_NormalizeFilter_H

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

/*! \brief Normalize normal map image filter.
 * \ingroup Drawing
 */
class T_DLLCLASS NormalizeFilter : public IImageFilter
{
	T_RTTI_CLASS;

protected:
	virtual void apply(Image* image) const T_OVERRIDE T_FINAL;
};
	
	}
}

#endif	// traktor_drawing_NormalizeFilter_H
