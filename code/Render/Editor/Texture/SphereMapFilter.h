#pragma once

#include "Drawing/IImageFilter.h"

namespace traktor
{
	namespace render
	{

class SphereMapFilter : public drawing::IImageFilter
{
	T_RTTI_CLASS;

protected:
	virtual void apply(drawing::Image* image) const override final;
};

	}
}

