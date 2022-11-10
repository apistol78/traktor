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

/*! Dilate filter.
 * \ingroup Drawing
 */
class T_DLLCLASS DilateFilter : public IImageFilter
{
	T_RTTI_CLASS;

public:
	explicit DilateFilter(int32_t iterations);

protected:
	virtual void apply(Image* image) const override final;

private:
	int32_t m_iterations;
};

}
