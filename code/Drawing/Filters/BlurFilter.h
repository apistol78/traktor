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

/*! Blur filter.
 * \ingroup Drawing
 */
class T_DLLCLASS BlurFilter : public IImageFilter
{
	T_RTTI_CLASS;

public:
	explicit BlurFilter(int32_t x, int32_t y);

protected:
	virtual void apply(Image* image) const override final;

private:
	int32_t m_x;
	int32_t m_y;
};

}
