#pragma once

#include "Terrain/Editor/IFallOff.h"

namespace traktor
{
	namespace drawing
	{

class Image;

	}

	namespace terrain
	{

class ImageFallOff : public IFallOff
{
	T_RTTI_CLASS;

public:
	ImageFallOff(const drawing::Image* image);

	virtual float evaluate(float x, float y) const override final;

private:
	Ref< const drawing::Image > m_image;
};

	}
}

