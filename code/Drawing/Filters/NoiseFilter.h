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

/*! Noise filter filter.
 * \ingroup Drawing
 */
class T_DLLCLASS NoiseFilter : public IImageFilter
{
	T_RTTI_CLASS;

public:
	explicit NoiseFilter(float strength);

protected:
	virtual void apply(Image* image) const override final;

private:
	float m_strength;
};

}
