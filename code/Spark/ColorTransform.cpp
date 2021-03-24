#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Spark/ColorTransform.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.ColorTransform", ColorTransform, ActionObjectRelay)

ColorTransform::ColorTransform()
:	ActionObjectRelay("flash.geom.ColorTransform")
,	mul(Color4f(1.0f, 1.0f, 1.0f, 1.0f))
,	add(Color4f(0.0f, 0.0f, 0.0f, 0.0f))
{
}

ColorTransform::ColorTransform(const ColorTransform& cxform)
:	ActionObjectRelay("flash.geom.ColorTransform")
,	mul(cxform.mul)
,	add(cxform.add)
{
}

ColorTransform::ColorTransform(const Color4f& mul_)
:	ActionObjectRelay("flash.geom.ColorTransform")
,	mul(mul_)
,	add(Color4f(0.0f, 0.0f, 0.0f, 0.0f))
{
}

ColorTransform::ColorTransform(const Color4f& mul_, const Color4f& add_)
:	ActionObjectRelay("flash.geom.ColorTransform")
,	mul(mul_)
,	add(add_)
{
}

void ColorTransform::serialize(ISerializer& s)
{
	s >> Member< Color4f >(L"mul", mul);
	s >> Member< Color4f >(L"add", add);
}

	}
}
