#ifndef traktor_drawing_DilateFilter_H
#define traktor_drawing_DilateFilter_H

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
	
/*! \brief Dilate filter.
 * \ingroup Drawing
 */
class T_DLLCLASS DilateFilter : public IImageFilter
{
	T_RTTI_CLASS;

public:
	DilateFilter(int32_t iterations);

protected:
	virtual void apply(Image* image) const;

private:
	int32_t m_iterations;
};
	
	}
}

#endif	// traktor_drawing_DilateFilter_H
