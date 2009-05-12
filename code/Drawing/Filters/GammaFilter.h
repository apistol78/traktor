#ifndef traktor_drawing_GammaFilter_H
#define traktor_drawing_GammaFilter_H

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
	
/*! \brief Gamma filter.
 * \ingroup Drawing
 */
class T_DLLCLASS GammaFilter : public ImageFilter
{
	T_RTTI_CLASS(GammaFilter)

public:
	GammaFilter(float gamma);

	GammaFilter(float gammaR, float gammaG, float gammaB, float gammaA = 1.0f);

protected:
	virtual Image* apply(const Image* image);

private:
	float m_gamma[4];
};
	
	}
}

#endif	// traktor_drawing_GammaFilter_H
