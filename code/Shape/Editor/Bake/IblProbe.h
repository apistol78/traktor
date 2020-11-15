#pragma once

#include "Core/Ref.h"
#include "Core/Object.h"

namespace traktor
{
	namespace drawing
	{

class Image;

	}

	namespace shape
	{
	
class IblProbe : public Object
{
	T_RTTI_CLASS;

public:
	explicit IblProbe(const drawing::Image* radiance);

	Color4f sample(const Vector4& direction) const;

private:
	Ref< const drawing::Image > m_radiance;
};
	
	}
}
