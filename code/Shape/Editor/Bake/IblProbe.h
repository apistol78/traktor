#pragma once

#include "Core/Ref.h"
#include "Core/Object.h"

namespace traktor
{
	namespace drawing
	{

class IImageFilter;
class Image;

	}

	namespace shape
	{
	
class IblProbe : public Object
{
	T_RTTI_CLASS;

public:
	explicit IblProbe(drawing::Image* radiance);

	Color4f sampleRadiance(const Vector4& direction) const;

	void apply(const drawing::IImageFilter* imageFilter);

	const drawing::Image* getRadianceImage() const { return m_radiance; }

private:
	Ref< drawing::Image > m_radiance;
};
	
	}
}
