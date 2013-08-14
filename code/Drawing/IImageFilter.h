#ifndef traktor_drawing_IImageFilter_H
#define traktor_drawing_IImageFilter_H

#include "Core/Object.h"

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
	
class Image;

/*! \brief Image filter base class.
 * \ingroup Drawing
 */
class T_DLLCLASS IImageFilter : public Object
{
	T_RTTI_CLASS;

protected:
	friend class Image;

	virtual void apply(Image* image) const = 0;
};
	
	}
}

#endif	// traktor_drawing_IImageFilter_H
