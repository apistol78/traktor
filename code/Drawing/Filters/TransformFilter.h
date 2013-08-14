#ifndef traktor_drawing_TransformFilter_H
#define traktor_drawing_TransformFilter_H

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

/*! \brief Transform colors filter.
 * \ingroup Drawing
 *
 * Linearly transform colors; RGBA' = RGBA * Km + Kc
 */
class T_DLLCLASS TransformFilter : public IImageFilter
{
	T_RTTI_CLASS;

public:
	TransformFilter(const Color4f& Km, const Color4f& Kc);

protected:
	virtual void apply(Image* image) const;

private:
	Color4f m_Km;
	Color4f m_Kc;
};
	
	}
}

#endif	// traktor_drawing_TransformFilter_H
