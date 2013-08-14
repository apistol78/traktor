#ifndef traktor_render_SphereMapFilter_H
#define traktor_render_SphereMapFilter_H

#include "Drawing/IImageFilter.h"

namespace traktor
{
	namespace render
	{

class SphereMapFilter : public drawing::IImageFilter
{
	T_RTTI_CLASS;

protected:
	virtual void apply(drawing::Image* image) const;
};

	}
}

#endif	// traktor_render_SphereMapFilter_H
