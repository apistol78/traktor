#ifndef traktor_drawing_BlurFilter_H
#define traktor_drawing_BlurFilter_H

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
	
/*! \brief Blur filter.
 * \ingroup Drawing
 */
class T_DLLCLASS BlurFilter : public IImageFilter
{
	T_RTTI_CLASS;

public:
	BlurFilter(int32_t x, int32_t y);

protected:
	virtual void apply(Image* image) const T_OVERRIDE T_FINAL;

private:
	int32_t m_x;
	int32_t m_y;
};
	
	}
}

#endif	// traktor_drawing_BlurFilter_H
