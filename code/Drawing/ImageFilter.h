#ifndef traktor_drawing_ImageFilter_H
#define traktor_drawing_ImageFilter_H

#include "Core/Object.h"
#include "Core/Heap/Ref.h"

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
	
class Image;

/*! \brief Image filter base class.
 * \ingroup Drawing
 */
class T_DLLCLASS ImageFilter : public Object
{
	T_RTTI_CLASS(ImageFilter)

protected:
	friend class Image;

	virtual Ref< Image > apply(const Image* image) = 0;
};
	
	}
}

#endif	// traktor_drawing_ImageFilter_H
