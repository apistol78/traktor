#include "Flash/Action/Common/ColorTransform.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ColorTransform", ColorTransform, ActionObjectRelay)

ColorTransform::ColorTransform(const SwfCxTransform& transform)
:	ActionObjectRelay("flash.geom.ColorTransform")
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
