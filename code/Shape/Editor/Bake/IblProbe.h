#pragma once

#include "Core/Ref.h"
#include "Shape/Editor/Bake/IProbe.h"

namespace traktor
{
	namespace drawing
	{

class Image;

	}

	namespace shape
	{
	
class IblProbe : public IProbe
{
	T_RTTI_CLASS;

public:
	explicit IblProbe(const drawing::Image* radiance, const drawing::Image* importance);

	virtual float getDensity(const Vector4& direction) const override final;

	virtual float getProbability(const Vector4& direction) const override final;

	virtual Color4f sample(const Vector4& direction) const override final;

private:
	Ref< const drawing::Image > m_radiance;
	Ref< const drawing::Image > m_importance;
};
	
	}
}
