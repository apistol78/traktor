#pragma once

#include "Drawing/IImageFilter.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DRAWING_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::drawing
{

/*! Gamma filter.
 * \ingroup Drawing
 */
class T_DLLCLASS GammaFilter : public IImageFilter
{
	T_RTTI_CLASS;

public:
	explicit GammaFilter(float gamma);

	explicit GammaFilter(float gammaR, float gammaG, float gammaB, float gammaA = 1.0f);

protected:
	virtual void apply(Image* image) const override final;

private:
	float m_gamma[4];
};

}
