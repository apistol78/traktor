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

/*! Normal map image filter.
 * \ingroup Drawing
 *
 * Calculate a normal map by sampling heights from
 * source image.
 */
class T_DLLCLASS NormalMapFilter : public IImageFilter
{
	T_RTTI_CLASS;

public:
	explicit NormalMapFilter(float scale);

protected:
	virtual void apply(Image* image) const override final;

private:
	float m_scale;
};

}
