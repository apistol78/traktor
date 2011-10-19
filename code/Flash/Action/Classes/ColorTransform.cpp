#include "Flash/Action/Classes/ColorTransform.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ColorTransform", ColorTransform, ActionObject)

ColorTransform::ColorTransform(const SwfCxTransform& transform)
:	ActionObject("flash.geom.Transform")
,	m_transform(transform)
{
}

void ColorTransform::setTransform(const SwfCxTransform& transform)
{
	m_transform = transform;
}

SwfCxTransform& ColorTransform::getTransform()
{
	return m_transform;
}

const SwfCxTransform& ColorTransform::getTransform() const
{
	return m_transform;
}

	}
}
