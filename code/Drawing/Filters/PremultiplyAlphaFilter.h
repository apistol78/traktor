#ifndef traktor_drawing_PremultiplyAlphaFilter_H
#define traktor_drawing_PremultiplyAlphaFilter_H

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

/*! \brief Multiply color with alpha filter.
 * \ingroup Drawing
 */
class T_DLLCLASS PremultiplyAlphaFilter : public IImageFilter
{
	T_RTTI_CLASS;

protected:
	virtual void apply(Image* image) const;
};
	
	}
}

#endif	// traktor_drawing_PremultiplyAlphaFilter_H
