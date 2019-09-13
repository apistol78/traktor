#pragma once

#include "Drawing/IImageFilter.h"

namespace traktor
{
	namespace drawing
	{

class SphereMapFilter : public IImageFilter
{
	T_RTTI_CLASS;

protected:
	virtual void apply(Image* image) const override final;
};

	}
}

