/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_drawing_GammaFilter_H
#define traktor_drawing_GammaFilter_H

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
	
/*! \brief Gamma filter.
 * \ingroup Drawing
 */
class T_DLLCLASS GammaFilter : public IImageFilter
{
	T_RTTI_CLASS;

public:
	GammaFilter(float gamma);

	GammaFilter(float gammaR, float gammaG, float gammaB, float gammaA = 1.0f);

protected:
	virtual void apply(Image* image) const T_OVERRIDE T_FINAL;

private:
	float m_gamma[4];
};
	
	}
}

#endif	// traktor_drawing_GammaFilter_H
